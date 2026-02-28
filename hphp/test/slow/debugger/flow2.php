<?hh

// Warning: line numbers are sensitive, do not change

function bar($a) {
  return $a + 2;
}

function genFoo($a) {
  $a = bar($a);
  $z = yield $a+5;
  yield HH\Lib\Legacy_FIXME\cast_for_arithmetic($z)+1;
  error_log('Finished in genFoo');
}

class C1 {
private
  $x = 0;

  public function __construct($a) {
    $x = $a;
    error_log('Constructor');
  }




}

// Test the following:
// - Stepping over instructions that cause destructors to run.
// - Stepping over instructions that call iterator methods.
// - Stepping out of destructors.
// - Stepping out of iterator methods.
function foo($a) {
  $c = new C1(5);

  $i = 0;
  foreach (genFoo($a) as $x) {
    $i++;
    var_dump($x);
  }

  $c = new C1(6); // Runs a destructor
  $d = $c;
  $e = new C1(7);
  $c = null;

  var_dump($d);
} // Ret runs two destructors

function test($a) {
  foo($a);
}

<<__EntryPoint>> function main() {
error_log('flow2.php done');
}
