<?hh //partial

class A {
  public function foo($x): void {
    bar($x);
  }
}

function bar(int $_): void {
}
