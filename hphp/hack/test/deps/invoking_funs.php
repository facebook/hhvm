<?hh

type C = int;

function A() : int {
    return 0;
}

function B() : C {
    return A();
}
