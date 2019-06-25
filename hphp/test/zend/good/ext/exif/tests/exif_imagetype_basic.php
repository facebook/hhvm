<?hh

/* Prototype  : int exif_imagetype  ( string $filename  )
 * Description: Determine the type of an image
 * Source code: ext/exif/exif.c */
<<__EntryPoint>> function main(): void {
echo "*** Testing exif_imagetype() : basic functionality ***\n";

var_dump(exif_imagetype(dirname(__FILE__).'/test2.jpg'));
echo "===Done===";
}
