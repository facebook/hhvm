<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

function test() :mixed{
  echo "Inside test() in ".__FILE__."\n";
  try {
    echo "\$v1: $v1, \$v2: $v2\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
<<__EntryPoint>>
function entrypoint_test(): void {
  error_reporting(-1);

  $local1 = 100;
  var_dump($local1);

  echo "====\n";
  print_r(get_included_files());
  echo "====\n";
}
