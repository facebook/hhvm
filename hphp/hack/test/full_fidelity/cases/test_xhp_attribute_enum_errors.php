<?hh //strict

class :Foo {
    attribute enum {} a; // error
    attribute enum {"xy", 'z', 123, 1.0} b; // legal
}
