<?hh
<<__EntryPoint>> function main(): void {
  error_reporting(0);

  $img = new Imagick();
  try {
    echo $img->foobar;
  } catch (UndefinedPropertyException $e) {
    echo "OK";
  }
}
