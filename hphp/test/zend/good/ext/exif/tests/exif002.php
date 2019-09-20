<?hh
/*
  test1.jpg is a 1*1 image that does not contain any Exif/Comment information
  test2.jpg is the same image but contains Exif/Comment information and a
            copy of test1.jpg as a thumbnail. */
<<__EntryPoint>> function main(): void {
$infile = dirname(__FILE__).'/test1.jpg';
$width = null;
$height = null;
$type = null;
echo md5_file($infile).'_'.filesize($infile);
$thumb = exif_thumbnail(dirname(__FILE__).'/test2.jpg', inout $width,
                        inout $height, inout $type);
echo " == ";
echo md5($thumb).'_'.strlen($thumb);
echo "\n";
}
