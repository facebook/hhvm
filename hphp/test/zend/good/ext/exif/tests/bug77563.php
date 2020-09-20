<?hh

<<__EntryPoint>>
function main(): void {
  $w = $h = $t = null;
  $s = exif_thumbnail(__DIR__.'/bug77563.jpg', inout $w, inout $h, inout $t);
  var_dump($s);
}
