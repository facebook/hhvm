<?hh

abstract class A {
    public function a() :mixed{ }
    private function b() :mixed{ }
    protected function c() :mixed{ }
}

class B extends A {
    private function bb() :mixed{ }

    static public function test() :mixed{
        var_dump(get_class_methods('A'));
        var_dump(get_class_methods('B'));
    }
}

<<__EntryPoint>> function main(): void {
var_dump(get_class_methods('A'));
var_dump(get_class_methods('B'));


B::test();
}
