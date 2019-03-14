<?hh // partial

class Foo {}
<<__RxShallow>>
function some_third_func(<<__OwnedMutable>>Foo $foo, bool $cond): void {
    // OK
    if ($cond) {
      some_other_func(HH\Rx\move($foo));
    } else {
    }
}

<<__Rx>>
function some_other_func(<<__OwnedMutable>>Foo $foo): void {}
