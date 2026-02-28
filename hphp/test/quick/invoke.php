<?hh

class Normal {
  public function __invoke() :mixed{
    echo "This is Normal\n";
    var_dump($this);
  }
}

class Weird2 {
  private function __invoke() :mixed{
    echo "This is Weird2\n";
    var_dump($this);
  }
}

class Weird3 {
  private function __invoke() :mixed{
    echo "This is Weird3\n";
    var_dump($this);
  }
}

class InvokeFailure {}

function invoke_it($x) :mixed{ $x(); }

<<__EntryPoint>> function main(): void {
  invoke_it(new Normal);
  invoke_it(new Weird2);
  invoke_it(new Weird3);
  echo "About to fail:\n";
  invoke_it(new InvokeFailure);
}
