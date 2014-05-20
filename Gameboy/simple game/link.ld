ENTRY(main);
SECTIONS
{
  . = 0x08000000;
  .text : { *(.text) }
  .data 0x03000000 :
	AT(ADDR(.text) + SIZEOF(.text))
	{ *(.data) }
  .bss : { *(.bss) }
 
  ship_image = ADDR(.text) + SIZEOF(.text);
  image_size = SIZEOF(.data);

}