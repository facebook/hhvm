<?hh

trait T1 {
    public abstract function foo();
}

trait T2 {
    public abstract function foo();
}

class C {
    use T1, T2;

    public function foo() {
        echo "C::foo() works.\n";
    }
}
<<__EntryPoint>> function main(): void {
$o = new C;
$o->foo();
}
