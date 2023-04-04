<?hh

class A {
  public function __construct(private int $b): void {}
}

function main(A $a): void {
  $a?->b;
}
