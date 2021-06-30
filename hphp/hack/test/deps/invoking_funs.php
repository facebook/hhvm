<?hh

type C = int;

function A()[read_globals] : int {
    return 0;
}

function B()[globals] : C {
    return A();
}
