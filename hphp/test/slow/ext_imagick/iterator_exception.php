<?hh
<<__EntryPoint>>
function main(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });

  $magick = new Imagick('magick:rose');
  $iterator = new ImagickPixelIterator($magick);

  try {
    $dummy = new ImagickPixelIterator(new ImagickPixel);
  } catch (Exception $ex) {
    echo "__construct\n";
  }

  try {
    $iterator->setIteratorRow(-1);
  } catch (Exception $ex) {
    echo "setIteratorRow\n";
  }

  echo "==DONE==\n";
}
