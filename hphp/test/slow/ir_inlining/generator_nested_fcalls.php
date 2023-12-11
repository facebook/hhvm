<?hh

class SecurityLogger {
  private $x, $y;

  public function setResource($x) :mixed{
    $this->x = $x;
    return $this;
  }

  public function setExtra($x) :mixed{
    $this->y = $x;
    return $this;
  }

  public function log() :mixed{
    echo "logging\n";
  }
}

function SecurityLogger($x, $y) :mixed{
  return new SecurityLogger();
}

function unique_function() :mixed{
  mt_rand();
  return vec['1'];
}

function foo() :AsyncGenerator<mixed,mixed,void>{
  $resource_arr = dict['a' => 'b'];
  // Inlining something which calls a function inside a generator
  // doesn't work.  This test shouldn't inline right now (if it does
  // it crashes).
  SecurityLogger(12, 12)
    ->setResource(darray(unique_function()))
    ->log();
  yield 12;
}

function main() :mixed{
  foreach (foo() as $k) {
}
}

<<__EntryPoint>>
function main_generator_nested_fcalls() :mixed{
main();
}
