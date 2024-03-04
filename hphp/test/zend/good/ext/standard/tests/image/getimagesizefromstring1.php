<?hh
<<__EntryPoint>> function main(): void {
$imagetype_filenames = dict[
  "GIF image file" => 'test.gif',
  "AVIF image file" => "test.avif",
  "HEIC image file" => "test.heic",
];
$img = __DIR__ . '/test.gif';
$info = null;

foreach ($imagetype_filenames as $key => $filename) {
  echo "\n-- $key ($filename) --\n";
  $img = __DIR__ . "/$filename";
  $i1 = getimagesize($img, inout $info);

  $data = file_get_contents($img);
  $i2 = getimagesizefromstring($data, inout $info);
  var_dump($i1);
  var_dump($i2);
};
echo "===DONE===\n";
}
