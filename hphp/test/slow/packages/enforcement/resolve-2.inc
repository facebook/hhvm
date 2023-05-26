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
