<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

function test()
{
    echo "Inside test() in " . __FILE__ . "\n";
    echo "\$v1: $v1, \$v2: $v2\n";
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
