<?hh

trait A {
    public function M1() :mixed{}
    public function M2() :mixed{}
}

trait B {
    public function M1() :mixed{}
    public function M2() :mixed{}
}

class MyClass {
    use A;
    use B;
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
}
