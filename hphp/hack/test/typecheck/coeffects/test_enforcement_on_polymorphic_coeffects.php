<?hh

class Foo {
    const ctx C = [write_props];

    public function __construct(
        public int $prop_int,
    ) {}

    public function error_polymorphic_1(Foo $foo)[self::C] : void {
        ++$foo->prop_int; // Error
    }
}

abstract class Bar {
    const ctx C = [write_props];
    abstract public function error_polymorphic_2(Foo $foo)[this::C]: void;
}

class Baz extends Bar {
    public function error_polymorphic_2(Foo $foo)[this::C]: void {
        ++$foo->prop_int; // Error
    }
}

function error_polymorphic_3(Foo $foo)[Foo::C] : void {
    ++$foo->prop_int; // Error
}

function error_dependent_1(Foo $foo)[$foo::C] : void {
    ++$foo->prop_int; // Error
}

function error_dependent_2(
    Foo $foo,
    ?(function()[_]: void) $f,
  )[ctx $f]: void {
    ++$foo->prop_int; // Error
}
