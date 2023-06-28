<?hh

trait foo {
    public function test() :mixed{ return 3; }
}
trait c {
    public function test() :mixed{ return 2; }
}

trait b {
    public function test() :mixed{ return 1; }
}

class bar {
    use foo, c, b;
}
<<__EntryPoint>> function main(): void {
$x = new bar;
var_dump($x->test());
}
