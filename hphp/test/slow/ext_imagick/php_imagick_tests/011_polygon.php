<?hh
<<__EntryPoint>> function main(): void {
$im = new Imagick();
$draw = new ImagickDraw();

try {
  $draw->polygon(vec[
          dict['x' => 1, 'y' => 2],
          dict['x' => 'hello', 'y' => vec[]]
          ]);

  echo "pass\n";

} catch (Exception $e) {
  echo "fail\n";
}

try {
  $draw->polygon(vec[vec[]]);
  echo "fail\n";
} catch (ImagickDrawException $e) {
  echo "pass\n";
}
}
