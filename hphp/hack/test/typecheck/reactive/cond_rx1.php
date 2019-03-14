<?hh // partial

class Foo {}

<<__RxShallow>>
function some_func(<<__OwnedMutable>>Foo $foo): void {
    // OK
    if (HH\Rx\IS_ENABLED) {
        some_other_func(HH\Rx\move($foo));
    } else {
    }
}

<<__Rx>>
function some_other_func(<<__OwnedMutable>>Foo $foo): void {}
