<?hh // strict

class Example {
  public function __clone(): string {
    echo("Clone was called");
    return "A";
  }
}

function test(): void {
  $test = new Example();
  $test->__clone();
}
