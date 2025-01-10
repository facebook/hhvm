<?hh

class Klass {
  public async function foo(vec<int> $vec): Awaitable<void> {
    $y = ExtraBuiltins\genInt();
    /*range-start*/
    $x1 = $y;
    $x2 = ExtraBuiltins\genInt()
    /*range-end*/
    echo $x1;
    echo await $x2;
  }
}
