// ssu etec3701 fall 2007

//make an (empty)  virtual disk file.
//syntax: mkdisk disk_file size_in_MB vbr_code_file

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int mkdisk(const char* fname, unsigned size_in_mb)
{
	//arguments:
	//		argv[1] = disk file
	//		argv[2] = size of disk, in MB
	unsigned size = size_in_mb;

	if( size == 0 ){
		printf("bad size\n");
		return 1;
	}

	remove(fname);

	FILE* fp = fopen(fname,"wb");
	if(!fp){
		printf("bad disk file\n");
		return 1;
	}


	fseek(fp,size*1024*1024-1,SEEK_SET);
	fputc(0,fp);
	fclose(fp);

	//printf("---mkdisk: made %d MB disk\n",size);

	return 0;
}




// ssu etec3701 fall 2007 jh


//mkpartition: make partition table: always one partition occupying whole disk.
//syntax: mkpartition hard_disk_image



#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "mbr.h"

using namespace std;

struct PTE{
	unsigned char bootable;		//0x80=yes, 0x00=no
	unsigned char shead;		//start head

	//to get cylinder and sector: bytes are:
	// CCssssss CCCCCCCC   (sector field has two bits of cylinder too)
	unsigned char ssector;		//start sector
	unsigned char scyl;		//start cylinder

	unsigned char type;		//partition type; 0xe = DOS/Windows FAT16

	unsigned char ehead;		//end head
	unsigned char esector,ecyl;	//end sector and cylinder

	unsigned start;			//LBA start sector
	unsigned size;			//num sectors

} __attribute__((packed));


int mkpartition(const char* fname)
{
	//argv[1] = filename

	FILE* fp = fopen(fname,"r+b");
	if(!fp){
		cerr << "Cannot open hard drive!\n";
		return 1;
	}
	fseek(fp,0,SEEK_END);
	unsigned sz = ftell(fp)/1024/1024;
	fseek(fp,0,SEEK_SET);

	PTE ptable[4];

	memset(ptable,0,sizeof(ptable));

	//Determine cyl/head/sector count
	//cyls must be a ten bit number
	//head is an 8 bit number
	//sector is a six bit number, but these start from one, not zero...

	int numheads;
	int numcyls;
	int numsects;

	//every head = 16MB
	numcyls = 1024;
	numsects = 32;

	//need to round down to nearest multiple of 16MB
	numheads = sz/16;


	if( numheads > 256 || numcyls > 1024 || numsects > 64 ){
		cerr << "Too big: chs=" << numcyls << ' ' << numheads << ' ' << numsects << "!\n";
		return 1;
	}

	//write to partition table entry 1
	ptable[0].bootable=0x80;
	ptable[0].shead = 1;
	ptable[0].ssector = 1;
	ptable[0].scyl = 0;

	ptable[0].type = 0xe;	//hardcode DOS type

	ptable[0].ehead = numheads-1;
	ptable[0].esector = (( (numcyls-1) & 0xff00 )>>2) | (numsects);
	ptable[0].ecyl = ( (numcyls-1) & 0xff) ;


	//always start on a track boundary
	ptable[0].start = numsects*( (rand()&7) + 1);
	ptable[0].size = sz*1024*1024/512-ptable[0].start;

	#if 0
		//write some debugging info
		cout << numheads << " heads\n";
		cout << numcyls << " cylinders\n";
		cout << numsects << " sectors per cylinder\n";
		cout << "Start C/H/S: " <<  (int)( ptable[0].scyl | ( (ptable[0].ssector<<2)&0xff00)) << "/"
			<< int(ptable[0].shead) << "/" << (int)(ptable[0].ssector & 0x3f)  << "*\n";
		cout << "End C/H/S: " <<  (int)( ptable[0].ecyl | ( ( int(ptable[0].esector)<<2)&0xff00)) << "/"
			<< int(ptable[0].ehead) << "/" << (int)(ptable[0].esector & 0x3f)  << "*\n";
		printf("start sec/size sec= %d/%d\n",ptable[0].start,ptable[0].size);
		cout << "(*=one based numbering)\n";
	#endif

	//write MBR code
	fwrite(mbrcode,1,sizeof(mbrcode),fp);
	if( sizeof(mbrcode) > 446 ){
		cout << "Warning! MBR code was truncated\n";
	}
	//write only the partition table
	fseek(fp,446,SEEK_SET);
	fwrite(ptable,1,64,fp);

	//write signature bytes
	fputc(0x55,fp);
	fputc(0xaa,fp);

	//cout << "---mkpartition: created one partition successfully\n";

	fclose(fp);
	return 0;
}
//jh ssu etec3701 au2007

//overlaymbr: overlay mbr code onto hard drive image without
//touching anything else.

//syntax: overlaymbr mbr_file disk_file

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
//#include "mbr.h"



int overlaymbr(const char* diskfile)
{
    if( sizeof(mbrcode) >= 446 ){
        cerr << "MBR too large\n";
        exit(1);
    }
	//now overlay all but the partition data, which starts at offset 446
	FILE* ofp = fopen(diskfile,"r+b");
	if(!ofp){
		fprintf(stderr,"Cannot open file %s\n",diskfile);
		return 1;
	}

	fwrite(mbrcode,1,446,ofp);

	//write signature at end
	fseek(ofp,510,SEEK_SET);
	fputc(0x55,ofp);
	fputc(0xaa,ofp);

	fclose(ofp);

	return 0;
}
//jh ssu etec3701 au 2007
////mkfs: format a DOS filesystem
//Input: file representing hard disk
//Output: writes filesystem to the first partition.

//syntax:
// mkfs hard_disk_image

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;

struct xxxxxPTE{
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


int mkfs(const char* fname)
{
	//argv[1] = filename
	FILE* fp = fopen(fname,"r+b");
	if(!fp){
		cerr << "Cannot open hard drive!\n";
		return 1;
	}

	char mbr[512];

	fread(mbr,1,512,fp);

	if(  mbr[510] != 0x55 || mbr[511] != (char) 0xaa ){
		cerr << "No valid partition table found\n";
		return 1;
	}


	PTE* ptable = (PTE*) (mbr + 446);


	int es = ptable[0].esector & 0x3f;

	//sectors per cluster
	int spc;

	//size of disk in megabytes
	int szm = ptable[0].size * 512/1024/1024 + 1;

	//compute sectors per cluster
	if( szm <= 16)		spc = 4;
	else if( szm <= 32)	spc = 1;	//(!)
	else if( szm <= 64)	spc = 2;
	else if( szm <= 128)	spc = 4;
	else if( szm <= 256)	spc = 8;
	else if( szm <= 512)	spc = 16;
	else if( szm <= 1024)	spc = 32;
	else if( szm <= 2048)	spc = 64;
	else			spc = 128;


	#if 0
		int ec = int(ptable[0].endcyl) | int( (ptable[0].endsect << int(2) ) & 0xff00);
		cerr << "Size of partition: " << szm << " MB\n";
		cerr << "Sectors per cluster = " << spc << "\n";
		cerr << "C/H/S: " << ec+1 << "/" << ptable[0].endhead+1 << "/" << es << "\n";
	#endif


	//create a vbr

	VBR vbr;
	memset(&vbr,0,sizeof(vbr));

	strncpy(vbr.oem,"mkfs    ",8);
	vbr.bytes_per_sector = 512;
	vbr.sectors_per_cluster = spc;
	vbr.vbr_sectors = 1;
	vbr.num_fats = 2;
    
    int nrde = 512-32*(rand()&3);
	vbr.num_root_dir_entries = nrde;
	if( szm < 32 )
		vbr.num_sectors_small = ptable[0].size;
	vbr.id = 0xf8;
        vbr.sectors_per_fat = (ptable[0].size / spc * 2) / 512;
        vbr.sectors_per_track = es;
        vbr.num_heads = ptable[0].ehead+1;
        vbr.first_sector = ptable[0].start;
	if( szm >= 32 )
		vbr.num_sectors_big = ptable[0].size;
	vbr.drive_number = 0x80;
        vbr.sig1 = 0x29;
        vbr.serial_number = 314159265;
        strncpy(vbr.label,"hello world",11);
        strncpy(vbr.fstype,"FAT16   ",8);


	//write vbr
	char zeros32[32];
	memset(zeros32,0,32);

	fseek(fp,ptable[0].start*512,SEEK_SET);
	for(int i=0;i<16;++i)
		fwrite(zeros32,1,32,fp);
	fseek(fp,ptable[0].start*512,SEEK_SET);
	fwrite(&vbr,1,sizeof(vbr),fp);
	fseek(fp,ptable[0].start*512+510,SEEK_SET);
	fputc(0x55,fp);
	fputc(0xaa,fp);

	//now write FAT 1
	unsigned pos1 = ftell(fp);
	for(int i=0;i<vbr.sectors_per_fat;++i){
		for(int i=0;i<16;++i)
			fwrite(zeros32,1,32,fp);
	}
	unsigned pos2 = ftell(fp);

	//mark first two clusters as used
	fseek(fp,pos1,SEEK_SET);
	fputc(0xf8,fp);
	fputc(0xff,fp);
	fputc(0xff,fp);
	fputc(0xff,fp);
	fseek(fp,pos2,SEEK_SET);


	//write FAT 2
	pos1 = ftell(fp);
	for(int i=0;i<vbr.sectors_per_fat;++i){
		for(int i=0;i<16;++i)
			fwrite(zeros32,1,32,fp);
	}
	pos2 = ftell(fp);
	//mark first two clusters as used
	fseek(fp,pos1,SEEK_SET);
	fputc(0xf8,fp);
	fputc(0xff,fp);
	fputc(0xff,fp);
	fputc(0xff,fp);
	fseek(fp,pos2,SEEK_SET);


	//write root directory
	for(int i=0;i<vbr.num_root_dir_entries;++i)
		fwrite(zeros32,1,32,fp);


	//cout << "---mkfs: created empty FAT16 filesystem\n";

	fclose(fp);
	return 0;
}

int overlayvbr(const char*, const char*);

int main(int argc, char* argv[]){
    const char* fname = argv[1];
    int size_in_mb = atoi(argv[2]);
    const char* vfname = argv[3];

    srand(time(0));
    if( mkdisk(fname,size_in_mb) ||
        mkpartition(fname) ||
        overlaymbr(fname) ||
        mkfs(fname) ||
        overlayvbr(fname,vfname) ){
        cerr << "Failure\n";
        return 1;
    }
    return 0;
}
//jh ssu au2007 etec3701

//overlayvbr: overlay DOS vbr code onto hard drive image without
//touching anything else (including vbr parameters)

// syntax: overlayvbr  hd_image vbr_file
// Overlays the vbr on top of the hard disk image

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>



int overlayvbr(const char* fname, const char* vfname)
{
	FILE* fp = fopen(vfname,"rb");
	fseek(fp,0,SEEK_END);
	if( ftell(fp) > 512 ){
		fprintf(stderr,"VBR is too big!\n");
		return 1;
	}

	fseek(fp,0,SEEK_SET);

	char vbr[512];
	memset(vbr,0,512);
	fread(vbr,1,512,fp);
	fclose(fp);

	FILE* ofp = fopen(fname,"r+b");

	//go to location where partition offset is located
	fseek(ofp,454,SEEK_SET);

	unsigned ns = 0;
	fread(&ns,4,1,ofp);

	//printf("VBR of partition 1 is at sector %d\n",ns);

	//now, ns has number of sectors before first sector of partition 1 on HD
	//seek to that spot, bytewise
	fseek(ofp,ns*512,SEEK_SET);

	//now write the vbr, but don't write volume parameters
	fwrite(vbr,1,3,ofp);
	fseek(ofp,ns*512+62,SEEK_SET);
	fwrite(vbr+62,1,448,ofp);

	fclose(ofp);

	return 0;
}
