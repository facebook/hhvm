<?php
/* Prototype  : string finfo_buffer(resource finfo, char *string [, int options [, resource context]])
 * Description: Return infromation about a string buffer. 
 * Source code: ext/fileinfo/fileinfo.c
 * Alias to functions: 
 */

$magicFile = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'magic';

$options = array(
	FILEINFO_NONE,
	FILEINFO_MIME,
);

$buffers = array(
	"Regular string here",
	"\177ELF",
	"\000\000\0001\000\000\0000\000\000\0000\000\000\0002\000\000\0000\000\000\0000\000\000\0003",
	"\x55\x7A\x6E\x61",
	"id=ImageMagick",
	"RIFFüîò^BAVI LISTv",
);

echo "*** Testing finfo_buffer() : variation functionality with oo interface ***\n";

foreach( $options as $option ) {
	$finfo = new finfo( $option, $magicFile );
	foreach( $buffers as $string ) {
		var_dump( $finfo->buffer( $string, $option ) );
	}
}

?>
===DONE===