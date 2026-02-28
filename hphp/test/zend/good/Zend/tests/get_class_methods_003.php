<?hh

interface A {
    function aa():mixed;
    function bb():mixed;
    static function cc():mixed;
}

class C {
    public function a() :mixed{ }
    protected function b() :mixed{ }
    private function c() :mixed{ }

    static public function static_a() :mixed{ }
    static protected function static_b() :mixed{ }
    static private function static_c() :mixed{ }
}

class B extends C implements A {
    public function aa() :mixed{ }
    public function bb() :mixed{ }

    static function cc() :mixed{ }

    public function __construct() {
        var_dump(get_class_methods('A'));
        var_dump(get_class_methods('B'));
        var_dump(get_class_methods('C'));
    }
}
<<__EntryPoint>> function main(): void {
new B;
}
