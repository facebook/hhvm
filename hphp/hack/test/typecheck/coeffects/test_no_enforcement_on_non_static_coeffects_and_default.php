<?hh

newctx Codegen as [write_props];

class Foo {
    const ctx C = [write_props];

    public function __construct(
        public int $prop_int,
    ) {}
}

function no_check_nonsense_coeffect(Foo $foo)[nonsense] : void {
    ++$foo->prop_int; // No error
}

function no_check_explicit_default(Foo $foo)[defaults] : void {
    ++$foo->prop_int; // No error
}

function no_check_encapsulated(Foo $foo)[Codegen] : void {
    ++$foo->prop_int; // No error
}
