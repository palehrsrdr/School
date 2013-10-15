// jh etec 3701 au 2008

//stdarg header to allow variable length arg list processing

#ifndef STDARG_H
#define STDARG_H


//this keeps track of where we were in the argument list:
//curr is a pointer to the next thing to return when someone
//calls va_arg.
typedef struct _va_list{
	char* curr;
} va_list ;

//initialize the list. ap is an item of type va_list.
//last is the name of the last parameter before an ellipses.
//This finds the address of that last parameter on the stack
//and then adds the size of that parameter so we now have
//ap.curr pointing to the first variable argument.
//Remember, C pushes arguments right-to-left
//so if we call f(a,b,c)
//they are pushed c, then b, then a.
//That means that they appear in memory as a,b,c (since the stack
//grows downward).
#define va_start( ap , last ) 						\
	ap.curr = ((char*)(&last)) + sizeof(last)

//no-op! But ANSI requires that we have it.
#define va_end( ap )

//magic with the comma operator!
//Assume that ap.curr is pointing to an item of type 'type'.
//Retrieve and return that item but also move ap up to the next
//item on the stack. If we separate two statements by a comma,
//the returned result is the last item listed. So we move ap.curr
//up by the correct number of bytes, then we find the previous location
//and return a pointer, casted to the correct type. Whew!
//If you seek to understand this, try running this C code:
//#include <stdio.h>
//int main(int argc, char* argv[])
//{
//	int a = 0;
//	int b = (a++ , 3);
//	printf("%d %d\n",a,b);
//}
//
//You will see the values 1 and 3.

#define va_arg( ap , type )						\
	( ap.curr += sizeof(type),*((type*)(ap.curr-sizeof(type))) )
	
	
#endif