<?hh // partial

class Foo {}
<<__RxShallow>>
function some_third_func(<<__OwnedMutable>>Foo $foo, bool $cond): void {
    if ($cond) {
        some_other_func(HH\Rx\move($foo));
    } else {
    }
    // ERROR: $foo might be unset
    $foo;
}

<<__Rx>>
function some_other_func(<<__OwnedMutable>>Foo $foo): void {}
