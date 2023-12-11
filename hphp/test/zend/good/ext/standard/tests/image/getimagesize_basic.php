<?hh
/* Prototype  : array getimagesize(string imagefile [, array info])
 * Description: Get the size of an image as 4-element array
 * Source code: ext/standard/image.c
 */
<<__EntryPoint>> function main(): void {
$imagetype_filenames = dict[
      // GIF file
      "GIF image file" => "200x100.gif",

      //JPEG file
      "JPEG image file" => "200x100.jpg",

      //PNG file
      "PNG image file" => "200x100.png",

      //SWF file
      "SWF image file" => "200x100.swf",

      //BMP file
      "BMP image file" => "200x100.bmp",

      //TIFF intel byte order
      "TIFF intel byte order image file" => "200x100.tif",

      //JPC file
      "JPC image file" => "test1pix.jpc",

      //JP2 file
      "JP2 image file" => "test1pix.jp2",

      //IFF file
      "IFF image file" => "test4pix.iff",

      "WEBP image file" => "200x100.webp",
];

echo "*** Testing getimagesize() : basic functionality ***\n";

$info = null;
// loop through each element of the array for imagetype
foreach($imagetype_filenames as $key => $filename) {
      echo "\n-- $key ($filename) --\n";
      var_dump( getimagesize(dirname(__FILE__)."/$filename", inout $info) );
      var_dump( $info );
};
echo "===DONE===\n";
}
