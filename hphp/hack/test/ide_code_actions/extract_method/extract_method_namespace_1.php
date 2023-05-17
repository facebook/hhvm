<?hh

async function genVoid(): Awaitable<void> {}

class Klass {
  public function foo(int $n): void {
    $awaitable = genVoid();
    /*range-start*/
    $x = $n * 2;
    $y = $awaitable;
    /*range-end*/
    $x + 2;
  }
}
