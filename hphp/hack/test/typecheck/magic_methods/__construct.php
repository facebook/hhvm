<?hh // strict

class Example {
  public function __construct(): void {
    echo("Construct was called");
  }
}

function test(): void {
  $test = new Example();
  $test->__construct();
}
