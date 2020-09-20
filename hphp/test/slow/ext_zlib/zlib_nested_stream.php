<?hh

function main() {
  $fnam = tempnam('/tmp', 'unittest');
  $f = fopen('compress.zlib://file://'.$fnam, 'w');
  fwrite($f, 'hello, world');
  fclose($f);

  $f = gzopen($fnam, 'r');
  var_dump(gzread($f, 1024));
  gzclose($f);
}


<<__EntryPoint>>
function main_zlib_nested_stream() {
main();
}
