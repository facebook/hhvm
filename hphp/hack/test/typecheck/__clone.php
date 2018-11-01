<?hh // strict

class Example {
  public function __clone(): string {
    echo("Clone was called");
  }
}

function test(): void {
  $test = new Example();
  $test->__clone();
}
