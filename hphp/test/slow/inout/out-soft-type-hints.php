<?hh

function foo($x, inout @int $y): string {
  return ($y = $x);
}

function bar($x, inout @int $y): void {
  $y = $x;
}

<<__EntryPoint>>
function main() {
  set_error_handler(
    ($errno, $errstr, $errfile, $errline) ==> {
      echo "[$errno] $errstr\n";
    }
  );

  $a = 'hello';
  $b = 42;
  $c = 58;
  foo($a, inout $b);
  foo($b, inout $c);
  var_dump($b, $c);
  bar(123, inout $c);
  var_dump($c);
}
