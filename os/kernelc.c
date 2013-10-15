#include "console.h"
#include "kprintf.h"
#include "util.h"


char* decl[] = {
"When in the Course of human events, it becomes necessary for one people to\n",
"dissolve the political bands which have connected them with another,\n",
"and to assume among the powers of the earth, the separate and equal station to\n",
"to which the Laws of Nature and of Nature's God entitle them, a decent\n",
"respect to the opinions of mankind requires that they should declare the\n",
"causes which impel them to the separation.\n",
"We hold these truths to be self-evident, that all men are created equal,\n",
"that they are endowed by their Creator with certain unalienable Rights, that\n",
"among these are Life, Liberty and the pursuit of Happiness.\n",
"\t--That to secure these rights, Governments are instituted among Men,\n",
" \t--That whenever any Form of Government becomes destructive of these\n",
"ends, it is the Right of the People to alter or to abolish it, and to\n",
"institute new Government, laying its foundation on such principles and\n",
"organizing its powers in such form, as to them shall seem most likely to\n",
"effect their Safety and Happiness.\n",
"\n",
"Prudence, indeed, will dictate that Governments long established should not\n",
"be changed for light and transient causes;and accordingly all experience hath",
"shewn, that mankind are more disposed to suffer, while evils are sufferable,\n",
"than to right themselves by abolishing the forms to which they are\n",
"accustomed. But when a long train of abuses and usurpations, pursuing\n",
"invariably the same Object evinces a design to reduce them under absolute Despotism,\n",
"it is their right, it is their duty, to throw off such Government, \n",
"and to provide new Guards for their future security.\n",
"   \t--Such has been the patient sufferance of these Colonies; and such is now the\n",
"necessity which constrains them to alter their former Systems of Government.",
""
};

int _main()
{	
	console_init();
	int i;
	for(i=0;decl[i][0];++i)
		kprintf("%d-%s",i,decl[i]);
	
	return 0;
}


int main()
{
	_main();
	return 0;
}