
<?hh

module a.b;

function baz1() {
    return foo<>;
}

function baz2() {
    return "foo";
}

function baz3() {
    return () ==> { return foo(); };
}

function baz4() {
    return Foo::foo<>;
}

function baz5() {
    return "Foo::foo";
}

function cbaz1() {
    return Foo::class;
}

function cbaz2() {
    return "Foo";
}

function cbaz3() {
    return () ==> { return Foo::class; };
}
