<?hh

function main() :mixed{
  $fnam = tempnam(sys_get_temp_dir(), 'unittest');
  $f = fopen('compress.zlib://file://'.$fnam, 'w');
  fwrite($f, 'hello, world');
  fclose($f);

  $f = gzopen($fnam, 'r');
  var_dump(gzread($f, 1024));
  gzclose($f);
}


<<__EntryPoint>>
function main_zlib_nested_stream() :mixed{
main();
}
