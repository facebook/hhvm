
<?hh

module z.z;

function soft_baz1() {
    return soft_foo<>;
}

function soft_baz2() {
    return "soft_foo";
}

function soft_baz3() {
    return () ==> { return soft_foo(); };
}

function soft_baz4() {
    return SoftFoo::static_foo<>;
}

function soft_baz5() {
    return "SoftFoo::static_foo";
}

function soft_cbaz1() {
    return SoftFoo::class;
}

function soft_cbaz2() {
    return "SoftFoo";
}

function soft_cbaz3() {
    return () ==> { return SoftFoo::class; };
}
