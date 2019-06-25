<?hh

trait TConstructor {
    public function foo() {
        echo "foo executed\n";
    }
    public function bar() {
        echo "bar executed\n";
    }
}

class OverridingIsSilent1 {
    use TConstructor {
        foo as __construct;
    }

    public function __construct() {
        echo "OverridingIsSilent1 __construct\n";
    }
}
<<__EntryPoint>> function main(): void {
$o = new OverridingIsSilent1;
}
