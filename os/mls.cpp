//jh ssu au 2007 etec3701

//mls: list directory info
//syntax: mls hard_disk_image

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <time.h>
#include <assert.h>
#include <iomanip>

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

void print_thousands(unsigned fbytes)
{
	int fb =  fbytes / 1000000000;
	int fm = (fbytes % 1000000000) / 1000000;
	int ft = (fbytes % 1000000) / 1000;
	int fh = (fbytes % 1000 );
	if( fbytes < 1000 )
		printf("%d" , fbytes );
	else if( fbytes < 1000000 )
		printf("%d,%03d", ft,fh );
	else if( fbytes < 1000000000 )
		printf("%d,%03d,%03d", fm,ft,fh);
	else
		printf("%d,%03d,%03d,%03d", fb,fm,ft,fh);
}

int main(int argc, char* argv[])
{
	//argv[1] = disk file

	char* diskfile = argv[1];
	
	FILE* fp = fopen(diskfile,"rb");
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
	
		
	//scan root directory looking for a file that matches
	//the one we're passing in. If found: reject (user must delete it first)
	fseek( fp, (ptable[0].start + vbr.vbr_sectors + vbr.num_fats * vbr.sectors_per_fat )* 512, SEEK_SET);

	const char* months[] = {"","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	
	for(int i=0;i<vbr.num_root_dir_entries;++i){
		DirEntry de;
		fread(&de,32,1,fp);
		if( de.name[0] != 0 && de.name[0] != (char) 0xe5 ){
			printf("%-8.8s %-3.3s ",de.name, de.ext);
			
			printf("%8d ", de.size );
			
			//printf("%02x " ,de.attrib);
			
			int hour = de.time >> 11;
			int minute = (de.time >> 5 ) & 0x3f;
			int sec = (de.time >> 11) & 0x1f;
			int year = (de.date >> 9) & 0x7f;
			int month = (de.date >> 5) & 0xf;
			int day = (de.date) & 0x1f;
			
			printf("%04d-%s-%02d %02d:%02d:%02d ",
				year+1980,months[month],day,hour,minute,sec);
			
			printf("\n");
		}
	}			
	
	printf("\n");
	
	//determine free space
	int fc=0;
	int uc=0;
	for(int i=0;i<65536;++i){
		if( fat[i] == 0 )
			++fc;
		else
			++uc;
	}
	
	unsigned fbytes = fc * vbr.sectors_per_cluster*512;
	print_thousands(fbytes);
	printf(" bytes free (%d clusters)\n", fc );
	
	//unsigned ubytes = uc * vbr.sectors_per_cluster*512;
	//print_thousands(ubytes);
	//printf(" bytes used (%d clusters)\n", uc );
	
	
	fclose(fp);
	
	return 0;
}
