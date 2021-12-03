<?hh

class Foo {

    public function __construct(
        public int $prop_int,
    ) {}

    public function pure_method()[] : void {
        $this->prop_int = 5; // write_props error
    }

    public function write_props_function()[write_props] : void {
        $this->prop_int = 5; // no error
    }

    public function write_this_props_function()[write_this_props]: void {
        $this->prop_int = 5; // no error
    }
}

function pure_function(Foo $x)[] : void {
    $x->prop_int = 5; // write_props error
}

function write_props_function(Foo $x)[write_props] : void {
    $x->prop_int = 5; // no error
}

function write_this_props_function(Foo $x)[write_this_props]: void {
    $x->prop_int = 5; // write_props error
}
