<?hh

class :Foo {
    attribute enum {} a; // error
    attribute enum {"xy", 'z', 123} b; // legal
    attribute enum {true, false} c; // error
    attribute enum {1.0} d; // error
}
