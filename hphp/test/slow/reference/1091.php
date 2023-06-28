<?hh

function test(inout $some_ref) :mixed{
  $some_ref = 42;
}
function test2($some_ref) :mixed{
  $some_ref = 42;
}

function run(inout $var, inout $some_ref) :mixed{
  $var = null;
  test(inout $var);
  var_dump($var);
  $var = null;
  test(inout $var);
  var_dump($some_ref, $var);
  test2($some_ref = 1);
  var_dump($some_ref);
  $var = null;
  test2($var);
  var_dump($var);
  $var = null;
  test2($some_ref = $var);
  var_dump($some_ref, $var);
}

<<__EntryPoint>>
function main() :mixed{
  $a = null;
  run(inout $a, inout $a);
}
