<?hh

interface I {
  public function f(): void;
}
interface J extends I {}

function test(J $j): void {
  if ($j is I) {
    $j->f();
  }
}
