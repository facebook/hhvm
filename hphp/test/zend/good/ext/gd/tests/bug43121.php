<?hh <<__EntryPoint>> function main(): void {
$im = imagecreate( 200, 100 );
$black = imagecolorallocate( $im, 0, 0, 0 );

$im_tile = imagecreatefromgif(dirname(__FILE__) . "/bug43121.gif" );
imagesettile( $im, $im_tile );
imagefill( $im, 0, 0, IMG_COLOR_TILED );

imagedestroy( $im );

print "OK";
}
