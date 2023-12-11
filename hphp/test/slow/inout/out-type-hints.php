<?hh

function foo($x, inout int $y): string {
  return ($y = $x);
}

function bar(inout int $x, inout int $y): int {
  list($x, $y) = vec[$y, $x];
  return $x + $y;
}

function main() :mixed{
  $a = 'hello';
  $b = 42;
  $c = 58;
  try { foo($a, inout $b); } catch (Exception $e) {}
  try { foo($b, inout $c); } catch (Exception $e) {}
  $r = bar(inout $b, inout $c);
  var_dump($r, $b, $c);
}


<<__EntryPoint>>
function main_out_type_hints() :mixed{
set_error_handler(($errno, $errstr, $errfile, $errline) ==> {
  echo "[$errno] $errstr\n";
  throw new Exception();
});

main();
}
