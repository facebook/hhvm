<?hh
class A {

    public function __construct() {
        $this -> foo();
    }

    private function foo() :mixed{
        echo __METHOD__ . "\r\n";
    }
}

class B extends A {
    public function foo() :mixed{
        echo __METHOD__ . "\r\n";
    }
}

class C extends A {
    protected function foo() :mixed{
        echo __METHOD__ . "\r\n";
    }
}

class D extends A {
        private function foo() :mixed{
                echo __METHOD__ . "\r\n";
        }
}
<<__EntryPoint>> function main(): void {
$a = new A();
$b = new B();
$c = new C();
$d = new D();
}
