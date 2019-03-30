<?hh

function foo($x, int &$y): void {
  $y = $x;
}

function bar($x, @int &$y): void {
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
  foo($a, &$b);
  var_dump($b);
  bar(42, &$b);
  var_dump($b);
}
