
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
