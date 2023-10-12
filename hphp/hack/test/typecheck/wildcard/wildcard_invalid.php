<?hh // strict

function foo(_ $x): void {
}

function bar(mixed $x): _ {
}

final class C {
  public function __construct(private _ $foo) {}
}
