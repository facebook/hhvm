<?hh

final class Foo {
  public function bar(string $x): void {
    echo $x;
  }

  public function foo(): void{
    $x = $this |> $$->bar<>;
    $x("Hello World!\n");
  }
}

<<__EntryPoint>>
function main(): void {
  $x = new Foo();
  $x->foo();
}
