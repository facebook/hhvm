<?hh

<<__EntryPoint>>
function main(): void {
  $t = null;
  $h = $t;
  $w = $h;
  $s = exif_thumbnail(__DIR__.'/bug77563.jpg', inout $w, inout $h, inout $t);
  var_dump($s);
}
