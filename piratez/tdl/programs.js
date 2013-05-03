/*
 * Copyright 2009, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//"use strict";

/**
 * @fileoverview This file contains objects to deal with WebGL
 *               programs.
 */

tdl.provide('tdl.programs');

tdl.require('tdl.log');
tdl.require('tdl.string');
tdl.require('tdl.webgl');


/**
 * A module for programs.
 * @namespace
 */
tdl.programs = tdl.programs || {};

/**
 * Loads a program from script tags.
 * @param {string} vertexShaderId The id of the script tag that contains the
 *     vertex shader source.
 * @param {string} fragmentShaderId The id of the script tag that contains the
 *     fragment shader source.
 * @return {tdl.programs.Program} The created program.
 */
tdl.programs.loadProgramFromScriptTags = function(
    vertexShaderId, fragmentShaderId) {
    var vertElem = document.getElementById(vertexShaderId);
    var fragElem = document.getElementById(fragmentShaderId);
    if (!vertElem) {
        throw("Can't find vertex program tag: " + vertexShaderId);
    }
    if (!fragElem) {
        throw("Can't find fragment program tag: " + fragmentShaderId);
    }
    return tdl.programs.loadProgram(
        document.getElementById(vertexShaderId).text,
        document.getElementById(fragmentShaderId).text);
};

tdl.programs.makeProgramId = function(vertexShader, fragmentShader) {
    return vertexShader + "/" + fragmentShader;
};

/**
 * Loads a program.
 * @param {string} vertexShader The vertex shader source.
 * @param {string} fragmentShader The fragment shader source.
 * @param {!function(error)) opt_asyncCallback. Called with
 *        undefined if success or string if failure.
 * @return {tdl.programs.Program} The created program.
 */
/*
tdl.programs.loadProgram = function(vertexShader, fragmentShader) {
  var id = tdl.programs.makeProgramId(vertexShader, fragmentShader);
  tdl.programs.init_();
  var program = gl.tdl.programs.programDB[id];
  if (program) {
    //if (opt_asyncCallback) {
    //  setTimeout(function() { opt_asyncCallback(); }, 1);
    //}
    return program;
  }
  try {
    program = new tdl.programs.Program(null,vertexShader, fragmentShader,true);
  } catch (e) {
    tdl.error(e);
    throw(e);
  }
  //if (!opt_asyncCallback) {
    gl.tdl.programs.programDB[id] = program;
  //}
  return program;
};
*/
/**
 * A object to manage a WebGLProgram.
 * @constructor
 * @param loader: A tdl.Loader object (unused if opts.params_are_source is true)
 * @param {string} vs_url The vertex shader url or source
 * @param {string} fs_url The fragment shader url or source.
 * @param opts: Optional: Can contain the following fields:
 *          * boolean params_are_source: If true, vs_url and fs_url are the actual 
 *              source code of the shader rather than a URL
 */
tdl.programs.Program = function(loader,vs_url,fs_url,opts){
    var that=this;
    
    if( opts === true )
        opts={params_are_source:true};
    else if( opts === false )
        opts={params_are_source:false};
    else if( opts === undefined )
        opts={};
    
    var params_are_source = (opts.params_are_source===undefined)?false:opts.params_are_source;
    
    if( opts.defines !== undefined ){
        //FIXME: This is not implemented yet
        this.defines = {};
        for(var k in opts.defines){
            this.defines[k+""]=""+opts.defines[k];
        }
    }
    else{
        this.defines={};
    }
    //var id = tdl.programs.makeProgramId(vs_url, fs_url);
    //this.id = id;
    tdl.programs.init_();
    //var program = gl.tdl.programs.programDB[id];
    //if (program) {
    //    return program;     //'new' returns 'program' instead of 'this'
    //                        //FIXME: Is this what we want?
    //}
    //gl.tdl.programs.programDB[id] = this;
    
    this.filedata={}
    
    if( params_are_source ){
        this.vs_url="$vs$";
        this.fs_url="$fs$";
        this.id = "(anonymous)";
        //they aren't url's -- they're the actual shader source
        this.filedata[this.vs_url]=vs_url;
        this.filedata[this.fs_url]=fs_url;
        this.finish_constructing_program();
    }
    else{
        this.vs_url = vs_url;
        this.fs_url = fs_url;
        this.id = vs_url+"+"+fs_url;
        if(!loader || loader.loadTextFiles === undefined ){
            throw(new Error("First argument to program constructor must be a Loader"));
        }
        
        var subloader = loader.createSubloader(this.finish_constructing_program.bind(this));
        
        subloader.loadTextFile(vs_url,
            function(txt){
                that.process_loaded_file(subloader,vs_url,txt);
            }
        );
        subloader.loadTextFile(fs_url,
            function(txt){
                that.process_loaded_file(subloader,fs_url,txt);
            }
        );
        subloader.finish();
    }
}

//Internal function: Fetch any #include'd files
tdl.programs.Program.prototype.process_loaded_file = function(loader,fname,fcontent){
    var rex=/^\s*#\s*include\s+["<]([^>"]+)/mg
    this.filedata[fname]=fcontent;
    var m = rex.exec(fcontent);
    var that=this;
    
    function make_callback(fn){
        return function(txtdata){
            that.process_loaded_file(loader,fn,txtdata);
        }
    }
    
    while( m ){
        var fn = m[1];
        if( this.filedata[fn] === undefined ){
            this.filedata[fn]="";
            loader.loadTextFile(fn, make_callback(fn));
        }
        m = rex.exec(fcontent);
    }
}

//Internal function: 
//replace #include <abc> with the actual text of the file abc
//rdepth is used to prevent infinite recursion
//Returns a triple: 
//  X,Y,Z
// where:
//      X[i] = line i of the source code, with includes expanded
//      Y[i] = the file where line i came from
//      Z[i] = the number of line i in its original source file
//The length of X,Y,Z are all equal
//FIXME: If rdepth is zero, we also do #define insertion
tdl.programs.Program.prototype.expand_includes = function(srcfilename,rdepth){
    
    if( rdepth > 25 ){
        throw new Error("Too many nested includes for "+this.vs_url+"+"+this.fs_url);
    }
        
    if( this.filedata[srcfilename] === undefined ){
        throw new Error("Internal error: Included file "+srcfilename+" was not included");
    }

    var sli = srcfilename.lastIndexOf("/");
    var pfx;
    if( sli === -1 )
        pfx="";
    else
        pfx = srcfilename.substr(0,pfx+1);
        
    var src = this.filedata[srcfilename];
    src=src.replace(/\r\n/g,"\n");
    src=src.replace(/\r/g,"\n");
    var L=src.split("\n");

    var L2=[];  //each line of code gets one entry here
    var FN=[];  //filename for each line of code: length of L2 === length of FN
    var OF=[];  //line number of L2 in its original file. Length of OF === length of L2
    
    var rex=/^\s*#\s*include\s+["<]([^>"]+)/
    for(var i=0;i<L.length;++i){
        var m = rex.exec(L[i]);
        if(m){
            var fname=m[1];
            var rv = this.expand_includes(pfx+fname,rdepth+1);
            var code = rv[0];
            var ff=rv[1];
            var oo=rv[2];
            for(var j=0;j<code.length;++j){
                L2.push(code[j]);
                FN.push(ff[j]);
                OF.push(oo[j]);
            }
        }
        else{
            L2.push(L[i]);
            FN.push(srcfilename);
            OF.push(i+1);
        }
    }
    return [L2,FN,OF];
}
    
tdl.programs.Program.prototype.finish_constructing_program = function() {
    

    var vssL = this.expand_includes(this.vs_url,0);
    var fssL = this.expand_includes(this.fs_url,0);
    
    var vss = vssL[0].join("\n");
    var fss = fssL[0].join("\n");
    
    
    var that = this;
    var program;

    
  /**
   * Loads a shader.
   * @param {!WebGLContext} gl The WebGLContext to use.
   * @param {string} shaderSource The shader source.
   * @param {number} shaderType The type of shader.
   * @return {!WebGLShader} The created shader.
   */
    var loadShader = function(gl, shaderSource, shaderType) {
        var shader = gl.createShader(shaderType);
        gl.shaderSource(shader, shaderSource);
        gl.compileShader(shader);
        var compiled = gl.getShaderParameter(shader, gl.COMPILE_STATUS);
        if (!compiled && !gl.isContextLost()) {
            // Something went wrong during compilation; get the error
            tdl.programs.lastError = gl.getShaderInfoLog(shader);
            gl.deleteShader(shader);
            //jh added vertex/fragment string
            msg="*** Error compiling " +
                (shaderType == gl.VERTEX_SHADER ? "vertex" : "fragment") +
                " shader '" + 
                (shaderType == gl.VERTEX_SHADER ? that.vs_url : that.fs_url)+"':\n";
            
            var rex=/\bERROR:\s+/
            rex.lastIndex=0;
            var EL=tdl.programs.lastError.split(rex);
            var i=0;
            for( var i=0;i<EL.length;++i){
                var rex2=/\d+:(\d+):/;
                var m = rex2.exec(EL[i]);
                if(!m )
                    msg += EL[i];
                else{
                    var linenum=parseInt(m[1],10)-1;
                    var w;
                    var o;
                    if( shaderType === gl.VERTEX_SHADER ){
                        w=vssL[1];
                        o=vssL[2];
                    }
                    else{
                        w=fssL[1];
                        o=fssL[2];
                    }
                    var truefname = w[linenum];
                    var trueline = o[linenum];
                
                    msg += "In file '"+truefname+"' at line "+trueline+": ";
                    msg += EL[i].substr(m.index+m[0].length);
                }
            }
            throw new Error(msg);
        }
        return shader
    };

    /**
    * Loads shaders
    * @param {!WebGLContext} gl The WebGLContext to use.
    * @param {string} vertexShader The vertex shader.
    * @param {string} fragmentShader The fragment shader.
    * @return {!WebGLProgram} The created program.
    */
    var loadProgram = function(gl, vertexShader, fragmentShader) {
        var e;
        var vs,fs,program;
        try {
            vs = loadShader(gl, vertexShader, gl.VERTEX_SHADER);
            fs = loadShader(gl, fragmentShader, gl.FRAGMENT_SHADER);
            program = gl.createProgram();
            gl.attachShader(program, vs);
            gl.attachShader(program, fs);
            linkProgram(gl, program);
        } catch (e) {
            if(vs)
                gl.deleteShader(vs);
            if(fs)
                gl.deleteShader(fs);
            if(program)
                gl.deleteProgram(program);
            throw(e);
        }
        return program;
    };


    /**
    * Links a WebGL program, throws if there are errors.
    * @param {!WebGLContext} gl The WebGLContext to use.
    * @param {!WebGLProgram} program The WebGLProgram to link.
    */
    var linkProgram = function(gl, program) {
        // Link the program
        gl.linkProgram(program);

        var linked = gl.getProgramParameter(program, gl.LINK_STATUS);
        if (!linked && !gl.isContextLost()) {
            // something went wrong with the link
            tdl.programs.lastError = gl.getProgramInfoLog (program);
            throw("*** Error in program linking for " + that.vs_url+"+"+that.fs_url+":"+tdl.programs.lastError);
        }
    };

    // Compile shaders
    this.vertexShaderSource=vss;
    this.fragmentShaderSource=fss;
    var program = loadProgram(gl, vss, fss);
    if (!program && !gl.isContextLost()) {
        throw ("could not compile program");
    }

    // TODO(gman): remove the need for this.
    function flatten(array){
        var flat = [];
        for (var i = 0, l = array.length; i < l; i++) {
            var type = Object.prototype.toString.call(array[i]).split(' ').pop().split(']').shift().toLowerCase();
            if (type) { 
                flat = flat.concat(/^(array|collection|arguments|object)$/.test(type) ? 
                    flatten(array[i]) : 
                    array[i]);
            }
        }
        return flat;
    }

    function createSetters(program) {
        // Look up attribs.
        var attribs = {
        };
        // Also make a plain table of the locs.
        var attribLocs = {
        };

        function createAttribSetter(info, index) {
            if (info.size != 1) {
                throw("arrays of attribs not handled");
            }
            return function(b) {
                gl.bindBuffer(gl.ARRAY_BUFFER, b.buffer());
                gl.enableVertexAttribArray(index);
                gl.vertexAttribPointer(
                    index, b.numComponents(), b.type(), b.normalize(), b.stride(), b.offset());
            };
        }

        that.uninitialized_attribs={};
        var numAttribs = gl.getProgramParameter(program, gl.ACTIVE_ATTRIBUTES);
        for (var ii = 0; ii < numAttribs; ++ii) {
            var info = gl.getActiveAttrib(program, ii);
    
            if (!info) {
                //jh changed
                continue;
                //break;
            }
      
            var name = info.name;
            if (tdl.string.endsWith(name, "[0]")) {
                name = name.substr(0, name.length - 3);
            }
            that.uninitialized_attribs[name]=true;
            var index = gl.getAttribLocation(program, info.name);
            attribs[name] = createAttribSetter(info, index);
            attribLocs[name] = index
        }

        // Look up uniforms
        var numUniforms = gl.getProgramParameter(program, gl.ACTIVE_UNIFORMS);
        var uniforms = {
        };
        var textureUnit = 0;

        //jh added for more sane uniform matrices so
        //we can use row-major matrices and everything
        //works like we'd expect
        function transpose2(x){
            var r = [];
            var m00 = m[0 * 4 + 0];
            var m01 = m[0 * 4 + 1];
            var m10 = m[1 * 4 + 0];
            var m11 = m[1 * 4 + 1];
            r[ 0] = m00;
            r[ 1] = m10;
            r[ 2] = m01;
            r[ 3] = m11;
            return r;
        }   
        function transpose3(m){
            var r = [];
            var m00 = m[0 * 4 + 0];
            var m01 = m[0 * 4 + 1];
            var m02 = m[0 * 4 + 2];
            var m10 = m[1 * 4 + 0];
            var m11 = m[1 * 4 + 1];
            var m12 = m[1 * 4 + 2];
            var m20 = m[2 * 4 + 0];
            var m21 = m[2 * 4 + 1];
            var m22 = m[2 * 4 + 2];
            r[ 0] = m00;
            r[ 1] = m10;
            r[ 2] = m20;
            r[ 3] = m01;
            r[ 4] = m11;
            r[ 5] = m21;
            r[ 6] = m02;
            r[ 7] = m12;
            r[ 8] = m22;
            return r;
        }
        function transpose4(m){
            var r = [];
            var m00 = m[0 * 4 + 0];
            var m01 = m[0 * 4 + 1];
            var m02 = m[0 * 4 + 2];
            var m03 = m[0 * 4 + 3];
            var m10 = m[1 * 4 + 0];
            var m11 = m[1 * 4 + 1];
            var m12 = m[1 * 4 + 2];
            var m13 = m[1 * 4 + 3];
            var m20 = m[2 * 4 + 0];
            var m21 = m[2 * 4 + 1];
            var m22 = m[2 * 4 + 2];
            var m23 = m[2 * 4 + 3];
            var m30 = m[3 * 4 + 0];
            var m31 = m[3 * 4 + 1];
            var m32 = m[3 * 4 + 2];
            var m33 = m[3 * 4 + 3];
            r[ 0] = m00;
            r[ 1] = m10;
            r[ 2] = m20;
            r[ 3] = m30;
            r[ 4] = m01;
            r[ 5] = m11;
            r[ 6] = m21;
            r[ 7] = m31;
            r[ 8] = m02;
            r[ 9] = m12;
            r[10] = m22;
            r[11] = m32;
            r[12] = m03;
            r[13] = m13;
            r[14] = m23;
            r[15] = m33;
            return r;
        }
        
        
        function createUniformSetter(info) {
          var loc = gl.getUniformLocation(program, info.name);
          var type = info.type;
          var name = info.name;
          var size = info.size;
          //console.log("uniform",name,"type=",type,"size=",size);
          
          function szcheck(v,nc){
              var le = v.length;
              if( le === undefined )
                le = 1;
              if( le !== nc*size )
                throw new Error("Setting uniform "+name+" in "+that.vs_url+"+"+that.fs_url+
                    ": Expected to get "+(nc*size)+" components, but got "+v.length);
                    //+" nc="+nc+" size="+size);
          }
              
          
          if (info.size > 1 && tdl.string.endsWith(info.name, "[0]")) {
            // It's an array.
            if (type == gl.FLOAT)
              return function() {
                return function(v) {
                    szcheck(v,1);
                    gl.uniform1fv(loc, v);
                };
              }();
            if (type == gl.FLOAT_VEC2)
              return function() {
                // I hope they don't use -1,-1 as their first draw
                //var old = new Float32Array([-1, -1]);
                return function(v) {
                    szcheck(v,2);
                    gl.uniform2fv(loc, v);
                };
              }();
            if (type == gl.FLOAT_VEC3)
              return function() {
                // I hope they don't use -1,-1,-1 as their first draw
                //var old = new Float32Array([-1, -1, -1]);
                return function(v) {
                    //jh
                  //if (v[0] != old[0] || v[1] != old[1] || v[2] != old[2]) {
                    szcheck(v,3);
                    gl.uniform3fv(loc, v);
                  //}
                };
              }();
            if (type == gl.FLOAT_VEC4)
              return function(v) { 
                  szcheck(v,4);
                  gl.uniform4fv(loc, v); 
              };
            if (type == gl.INT)
              return function(v) { 
                  szcheck(v,1)
                  gl.uniform1iv(loc, v); 
                  };
            if (type == gl.INT_VEC2)
              return function(v) { 
                  szcheck(v,2);
                  gl.uniform2iv(loc, v); 
                  };
            if (type == gl.INT_VEC3)
              return function(v) { 
                  szcheck(v,3);
                  gl.uniform3iv(loc, v); 
                  };
            if (type == gl.INT_VEC4)
              return function(v) { 
                  szcheck(v,4);
                  gl.uniform4iv(loc, v); 
                  };
            if (type == gl.BOOL)
              return function(v) { 
                  szcheck(v,1);
                  gl.uniform1iv(loc, v); 
                  };
            if (type == gl.BOOL_VEC2)
              return function(v) { 
                  szcheck(v,2);
                  gl.uniform2iv(loc, v); 
                  };
            if (type == gl.BOOL_VEC3)
              return function(v) { 
                  szcheck(v,3);
                  gl.uniform3iv(loc, v); 
                  };
            if (type == gl.BOOL_VEC4)
              return function(v) {
                  szcheck(v,4);
                   gl.uniform4iv(loc, v); 
                   };
            if (type == gl.FLOAT_MAT2)
              return function(v) { 
                  szcheck(v,4);
                  gl.uniformMatrix2fv(loc, false, transpose2(v) ); 
                  };
            if (type == gl.FLOAT_MAT3)
              return function(v) { 
                  szcheck(v,9);
                  gl.uniformMatrix3fv(loc, false, transpose3(v) ); 
                  };
            if (type == gl.FLOAT_MAT4)
              return function(v) { 
                  szcheck(v,16);
                  gl.uniformMatrix4fv(loc, false, transpose4(v) ); 
                  };
            if (type == gl.SAMPLER_2D || type == gl.SAMPLER_CUBE) {
                szcheck(v,1);
                var units = [];
                for (var ii = 0; ii < info.size; ++ii) {
                    units.push(textureUnit++);
                }
                return function(units) {
                    return function(v) {
                        gl.uniform1iv(loc, units);
                        v.bindToUnit(units);
                    };
                }(units);
            }
            throw ("unknown type: 0x" + type.toString(16));
          } else {
            if (type == gl.FLOAT)
              return function(v) {
                    szcheck(v,1);
                    gl.uniform1f(loc, v); 
                  };
            if (type == gl.FLOAT_VEC2)
              return function(v) { 
                    szcheck(v,2);
                    gl.uniform2fv(loc, v); 
                  };
            if (type == gl.FLOAT_VEC3)
              return function(v) { 
                    szcheck(v,3);
                    gl.uniform3fv(loc, v); 
                  };
            if (type == gl.FLOAT_VEC4)
                return function(v) {
                    szcheck(v,4);
                    gl.uniform4fv(loc, v);
                };
            if (type == gl.INT)
              return function(v) {
                  szcheck(v,1);
                   gl.uniform1i(loc, v);
                    };
            if (type == gl.INT_VEC2)
              return function(v) { 
                  szcheck(v,2);
                  gl.uniform2iv(loc, v);
                   };
            if (type == gl.INT_VEC3)
              return function(v) { 
                  szcheck(v,3);
                  gl.uniform3iv(loc, v);
                   };
            if (type == gl.INT_VEC4)
              return function(v) { 
                  szcheck(v,4);
                  gl.uniform4iv(loc, v);
                   };
            if (type == gl.BOOL)
              return function(v) { 
                  szcheck(v,1);
                  gl.uniform1i(loc, v); 
                  };
            if (type == gl.BOOL_VEC2)
              return function(v) { 
                  szcheck(v,2);
                  gl.uniform2iv(loc, v);
                   };
            if (type == gl.BOOL_VEC3)
              return function(v) { 
                  szcheck(v,3);
                  gl.uniform3iv(loc, v);
                   };
            if (type == gl.BOOL_VEC4)
              return function(v) {
                  szcheck(v,4);
                   gl.uniform4iv(loc, v);
                    };
            if (type == gl.FLOAT_MAT2)
              return function(v) { 
                  szcheck(v,4);
                  gl.uniformMatrix2fv(loc, false, transpose2(v) ); 
                  };
            if (type == gl.FLOAT_MAT3)
              return function(v) { 
                  szcheck(v,9);
                  gl.uniformMatrix3fv(loc, false, transpose3(v) ); 
                  };
            if (type == gl.FLOAT_MAT4)
              return function(v) { 
                  szcheck(v,16);
                  gl.uniformMatrix4fv(loc, false, transpose4(v) ); 
                  };
            if (type == gl.SAMPLER_2D || type == gl.SAMPLER_CUBE) {
                
                return function(unit) {

                    return function(v) {
                        szcheck(v,1);
                        gl.uniform1i(loc, unit);
                  
                        if( v.bindToUnit === undefined ){
                            throw new Error("You must pass a ***Texture*** object to setUniform('"+name+"',...)");
                        }
                        v.bindToUnit(unit);
                        var tmp = [v.width,v.height,1.0/v.width,1.0/v.height];
                        that.setUniform(name+"_size",tmp,true);
                        //console.log("Set",name+"_size","to",tmp,"for",v.name);
                    };
                } (textureUnit++);
            }
            throw ("unknown type: 0x" + type.toString(16));
          }
        }

        var textures = {};
        
        that.uninitialized_uniforms={};
        
        for (var ii = 0; ii < numUniforms; ++ii) {
            var info = gl.getActiveUniform(program, ii);
            if (!info) {
                //jh changed from break to continue
              continue; //break;
            }
            name = info.name;
            if (tdl.string.endsWith(name, "[0]")) {
                name = name.substr(0, name.length - 3);
            }
            that.uninitialized_uniforms[name]=true;
            var setter = createUniformSetter(info);
            uniforms[name] = setter;
            if (info.type == gl.SAMPLER_2D || info.type == gl.SAMPLER_CUBE) {
                textures[name] = setter;
            }
        }

        that.textures = textures;
        that.attrib = attribs;
        that.attribLoc = attribLocs;
        that.uniform = uniforms;
    }
    
    createSetters(program);

    this.loadNewShaders = function(vertexShaderSource, fragmentShaderSource) {
        var program = loadProgram(gl, vertexShaderSource, fragmentShaderSource);
        if (!program && !gl.isContextLost()) {
            throw ("could not compile program");
        }   
        that.program = program;
        createSetters();
    };

    this.program = program;
    this.good = this.asyncCallback ? false : true;

    var checkLater = function() {
        var e;
        try {
            checkShader(vs);
            checkShader(fs);
            checkProgram(program);
        } catch (e) {
            that.asyncCallback(e.toString());
            return;
        }
        gl.tdl.programs.programDB[that.programId] = this;
        that.asyncCallback();
    };
    if (this.asyncCallback) {
        setTimeout(checkLater, 1000);
    }
};

tdl.programs.handleContextLost_ = function() {
  if (gl.tdl && gl.tdl.programs && gl.tdl.programs.shaderDB) {
    delete gl.tdl.programs.shaderDB;
    delete gl.tdl.programs.programDB;
  }
  delete this.compileok
};

tdl.programs.init_ = function() {
  if (!gl.tdl.programs) {
    gl.tdl.programs = { };
    tdl.webgl.registerContextLostHandler(gl.canvas, tdl.programs.handleContextLost_, true);
  }
  if (!gl.tdl.programs.shaderDB) {
    gl.tdl.programs.shaderDB = { };
    gl.tdl.programs.programDB = { };
  }
};

tdl.programs.Program.prototype.use = function() {
  //if(!this.compileok){
  //    throw new Error("You must load and compile the program before you can use() it");
  //}
  gl.useProgram(this.program);
  gl.tdl.currentProgram = this; //also used in framebuffers.js (and tdl.js and webgl.js?)
};

//function dumpValue(msg, name, value) {
//  var str;
//  if (value.length) {
//      str = value[0].toString();
//     for (var ii = 1; ii < value.length; ++ii) {
//       str += "," + value[ii];
//     }
//  } else {
//    str = value.toString();
//  }
//  tdl.log(msg + name + ": " + str);
//}


tdl.programs.Program.prototype.setUniform = function(uniform, value, ignoremissing) {
  //jh added
  if( value === undefined ){
      console && console.trace && console.trace();
      throw(new Error("Cannot set uniform '"+uniform+"' to 'undefined'"));
  }
  
  if( gl.tdl.currentProgram !== this ){
      console.trace();
      throw new Error("You must use() the shader before setting uniforms");
  }
  
  if( this.uninitialized_uniforms[uniform] !== undefined ){
      delete this.uninitialized_uniforms[uniform];
  }
  
  var func = this.uniform[uniform];
  if (func) {
    func(value);
  }
  else{
      // jh added for debugging
      if( this.warned === undefined )
        this.warned={};
      if( this.warned[uniform] === undefined && ignoremissing !== true){
          this.warned[uniform] = true;
          tdl.log("Warning: Shader "+this.id+" doesn't have uniform '"+uniform+"'");
          //console && console.trace && console.trace();
      }
  }
};

//jh added to help debugging
tdl.programs.Program.prototype.getAttribLoc = function(name){
    var lo = this.attribLoc[name];
    if( lo === undefined ){
        if( this.awarned === undefined )
            this.awarned={};
        if( this.awarned[name] === undefined ){
            this.awarned[name]=true;
            tdl.log("Warning: Shader "+this.id+" doesn't have attribute '"+name+"'");
            //if( console && console.trace )
            //    console.trace();
        }
    }
    return lo;
};

//jh added
tdl.programs.Program.prototype.setAttribute = function(name,size,type,stride,offset){
    var idx = this.getAttribLoc(name);
    if( idx === undefined )
        return;
    gl.vertexAttribPointer(idx,size,type,false,stride,offset);
    gl.enableVertexAttribArray(idx);
    if( name in this.uninitialized_attribs )
        delete this.uninitialized_attribs[name];
}
    
//jh added to turn off all attributes
tdl.programs.Program.prototype.disableAllAttribs = function(){
    if( this.max_vertex_attribs === undefined ){
        this.max_vertex_attribs = gl.getParameter(gl.MAX_VERTEX_ATTRIBS);
    }
    for(var i=0;i<this.max_vertex_attribs;++i){
        gl.disableVertexAttribArray(i);
    }
}

//jh added to help debugging
tdl.programs.Program.prototype.checkUninitialized = function(){
    if( !this.warned_uninitialized ){
        var do_trace=false;
        this.warned_uninitialized=true;
        for(var X in this.uninitialized_uniforms ){
            tdl.log("Warning: Uninitialized uniform '"+X+"' in "+this.vs_url+"+"+this.fs_url);
            do_trace=true;
        }
        for(var X in this.uninitialized_attribs ){
            tdl.log("Warning: Uninitialized attrib '"+X+"' in "+this.vs_url+"+"+this.fs_url);
            do_trace=true;
        }
        if(do_trace)
            console.trace();
    }
}

//jh added to make it more DX like
//arguments:
//  size of vertex in bytes
//  n repetitions of:
//      attribute name: string
//      num components: 1,2,3,4
//      attribute type: ex: gl.FLOAT
//Items must be tightly packed (use an attribute name of "" to
//skip over data)
tdl.programs.Program.prototype.setVertexFormat = function(){

    if( this.max_vertex_attribs === undefined ){
        this.max_vertex_attribs = gl.getParameter(gl.MAX_VERTEX_ATTRIBS);
    }

    var AA=[];
    var offset=0;
    
    for(var i=0;i<arguments.length;i+=3){
        var name = arguments[i];
        var components = arguments[i+1];
        var type=arguments[i+2];
        
        if( name === undefined || type === undefined || components === undefined )
            throw new Error("Error in setVertexFormat");
            
        if( name !== "" && (components < 1 || components > 4 ) )
            throw new Error("Bad number of components in setVertexFormat: "+components);

        if( name !== "" ){
            var vi = this.getAttribLoc(name);
            if( vi !== undefined && vi !== null && vi !== -1)
                AA[vi]=[name,components,type,offset];
            if( name in this.uninitialized_attribs )
                delete this.uninitialized_attribs[name];
        }
        
        var sz;
        if( type === gl.FIXED || type === gl.FLOAT || type === gl.INT)
            sz=4;
        else if( type === gl.SHORT || type == gl.UNSIGNED_SHORT)
            sz=2;
        else if( type === gl.BYTE || type == gl.UNSIGNED_BYTE )
            sz=1;
        else
            throw new Error("Bad type to setVertexFormat:"+type);
            
        offset += components*sz;
    }     

    var vsize = offset;
    
    for(var i=0;i<this.max_vertex_attribs;++i){
        if( AA[i] ){
            gl.vertexAttribPointer(i,
                AA[i][1],AA[i][2],false,vsize,AA[i][3]);
        }
        
        if( AA[i]  &&  !gl.tdl.attribArrayStatus[i] )
            gl.enableVertexAttribArray(i);
        else if( !AA[i]  && gl.tdl.attribArrayStatus[i])
            gl.disableVertexAttribArray(i);
        //else if( AA[i] && gl.tdl.attribArrayStatus[i]) {}
        //else if( !AA[i] && !gl.tdl.attribArrayStatus[i]) {}
    }
}
