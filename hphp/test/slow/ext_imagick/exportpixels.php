<?hh
// height 2+(1<<52) makes width*height*bpp overflow int64.
<<__EntryPoint>>
function main(): mixed {
  $im = new Imagick();
  $im->newImage(64, 64, 'white');
  try {
    $im->exportImagePixels(0, 0, 4096, 2 + (1 << 52), 'I', Imagick::PIXEL_CHAR);
    echo "no exception\n";
  } catch (ImagickException $e) {
    echo "caught exception\n";
  }
}
