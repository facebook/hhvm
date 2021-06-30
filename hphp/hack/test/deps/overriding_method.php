<?hh

class B {
    public function foo()[write_props]: int {
        return 0;
    }
}

class C extends B {
    <<__Override>>
    public function foo()[]: int {
        return 1;
    }
}

function f1()[]: int {
    return 0;
}

function f2()[]: int {
    return f1();
}
