<?hh // strict

function f(inout int $i): void {}

class :x:foo {
  attribute int attr @required;

  public function test(): void {
    f(inout $this->:attr);
  }
}
