//ssu jh etec3701 au 2007
//revised au 2012
//mcp: copy a file to a virtual hard disk (FAT-16 format)

//syntax: mcp hard_disk_image source_filename dest_filename

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <list>
#include <time.h>
#include <assert.h>
#include <algorithm>

using namespace std;

struct PTE{
	unsigned char bootable;
	unsigned char starthead;

	//to get cs: bytes are:
	// CCssssss CCCCCCCC
	unsigned char startsect;
	unsigned char startcyl;

	unsigned char type;

	unsigned char endhead;
	unsigned char endsect,endcyl;


	unsigned start;
	unsigned size;
} __attribute__((packed));

struct VBR
{
        unsigned char           jmp[3];
        char                    oem[8];
        unsigned short          bytes_per_sector;
        unsigned char           sectors_per_cluster;
        unsigned short          vbr_sectors;
        unsigned char           num_fats;
        unsigned short          num_root_dir_entries;
        unsigned short          num_sectors_small;
        unsigned char           id;
        unsigned short          sectors_per_fat;
        unsigned short          sectors_per_track;
        unsigned short          num_heads;
        unsigned int            first_sector;
        unsigned int            num_sectors_big;
        unsigned char           drive_number;
        unsigned char           reserved;
        unsigned char           sig1;
        unsigned int            serial_number;
        char                    label[11];
        char                    fstype[8];
} __attribute__((packed));

struct DirEntry{
	char name[8];
	char ext[3];
	unsigned char attrib;
	char reserved[10];
	unsigned short time;
	unsigned short date;
	unsigned short start;
	unsigned int size;
} __attribute__((packed));

void clearbuff(char* x, int cs){
    for(int i=0;i<cs;++i){
        x[i]= 65+(i%26);
    }
}

int main(int argc, char* argv[])
{
	//argv[1] = disk file
	//argv[2] = input filename
	//argv[3] = destination filename

	if( argc != 4 ){
		cout << "Syntax: " << argv[0] << " disk_image in_file out_file\n";
		return 1;
	}

	char* diskfile = argv[1];
	char* infile = argv[2];
	char* outfile = argv[3];

	FILE* fp = fopen(diskfile,"r+b");
	if(!fp){
		cout << "Cannot open hard drive!\n";
		return 1;
	}

	//get partition table
	char mbr[512];
	fread(mbr,1,512,fp);
	PTE* ptable = (PTE*) (mbr + 446);

	//get volume boot record
	fseek( fp, ptable[0].start * 512 , SEEK_SET);
	VBR vbr;
	fread(&vbr,1,sizeof(vbr),fp);


	//read in FAT
	unsigned short* fat = new unsigned short[65536];
	assert(fat);
	memset(fat,0xff,65536*2);
	fseek( fp, (ptable[0].start + vbr.vbr_sectors )* 512, SEEK_SET);
	fread(fat,1,vbr.sectors_per_fat*512,fp);


	//split destination filename into base + ext
	char base[8];
	char ext[3];

	char* p = strstr(outfile,".");		//pointer to dot or end of string
	if( !p ){
		p = outfile+strlen(outfile);
	}

	char* q;			//current character
	int i = 0;			//counter for destination location
	for(q=outfile; q!=p; ++q){
		if( i == 8 ){
			cout << "Destination filename too long\n";
			return 1;
		}
		if( !isalnum(*q) ){
			cout << "Illegal character in destination filename\n";
			return 1;
		}
		base[i] = toupper(*q);
		++i;
	}
	while( i < 8 )				//pad with spaces
		base[i++] = ' ';
	i=0;
	if( *p ){
		for(q=p+1;*q!=0;++q){
			if( i == 3 ){
				cout << "Destination filename too long\n";
				return 1;
			}
			if( !isalnum(*q) ){
				cout << "Illegal character in destination filename\n";
				return 1;
			}
			ext[i] = toupper(*q);
			++i;
		}
	}
	while(i<3)				//pad with spaces
		ext[i++] = ' ';


	//scan root directory looking for a file that matches
	//the one we're passing in. If found: reject (user must delete it first)
	fseek( fp, (ptable[0].start + vbr.vbr_sectors + vbr.num_fats * vbr.sectors_per_fat )* 512, SEEK_SET);

	for(int i=0;i<vbr.num_root_dir_entries;++i){
		DirEntry de;
		fread(&de,32,1,fp);
		if( strncmp(de.name,base,8) == 0 && strncmp(de.ext,ext,3) == 0){
			cout << "Cannot overwrite existing file\n";
			printf("%.8s.%.3s\n",de.name,de.ext);
			return 1;
		}
	}

	//find a vacant directory entry
	fseek( fp, (ptable[0].start + vbr.vbr_sectors + vbr.num_fats * vbr.sectors_per_fat )* 512, SEEK_SET);
	int direntryloc=0;
    int direntryidx=-1;
	for(int i=0;i<vbr.num_root_dir_entries;++i){
		DirEntry de;
		fread(&de,32,1,fp);
		if( de.name[0] == 0 || (unsigned char)de.name[0] == 0xe5 ){
			//this is the one to use
			direntryloc = ftell(fp)-32;
            direntryidx = i;
			break;
		}
	}

	if( direntryidx == -1 ){
		cout << "No space in root directory!\n";
		return 1;
	}

	//start copying!
	FILE* ifp = fopen(infile,"rb");
	if(!ifp ){
		cout << "Cannot open input file " << infile << "\n";
		return 1;
	}

    //assemble list of free clusters
    vector<int> fc;
    for(int i=65535;i>=2;--i){
        if( fat[i] == 0 )
            fc.push_back(i);
    }
    if( direntryidx != 0 )
        random_shuffle(fc.begin(),fc.end());

	int csize = vbr.sectors_per_cluster * 512;
	char* buf = new char[csize];
	int rv;
    clearbuff(buf,csize);
	rv = fread(buf,1,csize,ifp);
	int first_cluster = 0;
	int prev_cluster = -1;
	int ok = 0;
	while( rv > 0 ){
        if( fc.empty() ){
            cout << "Not enough space on disk!";
            return 1;
        }

        int i=fc.back();
        fc.pop_back();
        //available!

        //mark as used. side effect: also handles EOF!
        fat[i] = 0xffff;

        if( prev_cluster != -1 )
            fat[prev_cluster] = i;
        else
            first_cluster = i;

        prev_cluster = i;

        int offs = (ptable[0].start + vbr.vbr_sectors +
						vbr.num_fats * vbr.sectors_per_fat +
						vbr.num_root_dir_entries * 32 / 512 )* 512;
        offs += (i-2) * vbr.sectors_per_cluster * 512;

        fseek( fp, offs , SEEK_SET);

        //write full cluster, even if we didn't read all of it
        fwrite(buf,1,csize,fp);

        clearbuff(buf,csize);
		rv = fread(buf,1,csize,ifp);
	}


	//write dir entry
	DirEntry de;
	memset(&de,0,sizeof(de));
	memcpy(de.name,base,8);
	memcpy(de.ext,ext,3);
	de.attrib = 0;

	time_t now;
	time(&now);
	struct tm* ltime = localtime( &now );
	de.time = (ltime->tm_sec / 2) | (ltime->tm_min<<5) | (ltime->tm_hour<<11);
	de.date = (ltime->tm_mday) | (ltime->tm_mon<<5) | ( (ltime->tm_year-80) << 9);
	de.start = first_cluster;
	de.size = ftell(ifp);

	fseek(fp,direntryloc,SEEK_SET);
	fwrite(&de,32,1,fp);


	//write fats
	fseek( fp, (ptable[0].start + vbr.vbr_sectors )* 512, SEEK_SET);
	fwrite(fat,1,vbr.sectors_per_fat*512,fp);

	fseek( fp, (ptable[0].start + vbr.vbr_sectors + vbr.sectors_per_fat )* 512, SEEK_SET);
	fwrite(fat,1,vbr.sectors_per_fat*512,fp);


	fclose(fp);
	fclose(ifp);

	return 0;
}
