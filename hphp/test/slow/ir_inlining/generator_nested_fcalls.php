<?hh

class SecurityLogger {
  private $x, $y;

  public function setResource($x) {
    $this->x = $x;
    return $this;
  }

  public function setExtra($x) {
    $this->y = $x;
    return $this;
  }

  public function log() {
    echo "logging\n";
  }
}

function SecurityLogger($x, $y) {
  return new SecurityLogger();
}

function unique_function() {
  mt_rand();
  return varray['1'];
}

function foo() {
  $resource_arr = darray['a' => 'b'];
  // Inlining something which calls a function inside a generator
  // doesn't work.  This test shouldn't inline right now (if it does
  // it crashes).
  SecurityLogger(12, 12)
    ->setResource(darray(unique_function()))
    ->log();
  yield 12;
}

function main() {
  foreach (foo() as $k) {
}
}

<<__EntryPoint>>
function main_generator_nested_fcalls() {
main();
}
