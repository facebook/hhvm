<?hh
<<__EntryPoint>> function main(): void {
$im = new Imagick();
$draw = new ImagickDraw();

try {
  $draw->polygon(varray[
          darray['x' => 1, 'y' => 2],
          darray['x' => 'hello', 'y' => array()]
          ]);

  echo "pass\n";

} catch (Exception $e) {
  echo "fail\n";
}

try {
  $draw->polygon(varray[array()]);
  echo "fail\n";
} catch (ImagickDrawException $e) {
  echo "pass\n";
}
}
