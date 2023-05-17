<?hh

class Klass {
  public async function foo(int $x): AsyncIterator<int> {
    /*range-start*/
    await gen_void();
    $y = $x;
    yield 3;
    /*range-end*/
    $z = $y
  }
}
