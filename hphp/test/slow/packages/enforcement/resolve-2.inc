<?hh

module x.y;

function bar1() {
    return foo<>;
}

function bar2() {
    return "foo";
}

function bar3() {
    return () ==> { return foo(); };
}

function bar4() {
    return Foo::foo<>;
}

function bar5() {
    return "Foo::foo";
}

function cbar1() {
    return Foo::class;
}

function cbar2() {
    return "Foo";
}

function cbar3() {
    return () ==> { return Foo::class; };
}
