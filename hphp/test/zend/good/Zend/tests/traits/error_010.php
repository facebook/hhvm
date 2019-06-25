<?hh

trait foo {
    public function test() { return 3; }
}
trait c {
    public function test() { return 2; }
}

class bar {
    use foo, c { c::test insteadof foo; }
    use foo, c { c::test insteadof foo; }
}
<<__EntryPoint>> function main(): void {
$x = new bar;
var_dump($x->test());
}
