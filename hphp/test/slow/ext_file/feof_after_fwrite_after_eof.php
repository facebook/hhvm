<?hh

function main() :mixed{
  $inputs = vec[
    'php://temp',
    'php://memory',
  ];

  foreach ($inputs as $input) {
    printf("---%s---\n", $input);
    $f = fopen($input, 'r+');
    var_dump(fseek($f, 5, SEEK_SET));
    var_dump(ftell($f));
    fread($f, 1);
    var_dump(feof($f));
    fwrite($f, 'foo');
    var_dump(feof($f));
    fseek($f, 0, SEEK_SET);
    fwrite($f, "1234567890");
    var_dump(feof($f));
    ftruncate($f, 0);
    fwrite($f, "abcdefghij");
    rewind($f);
    var_dump(fread($f, 10));
  }
}


<<__EntryPoint>>
function main_feof_after_fwrite_after_eof() :mixed{
main();
}
