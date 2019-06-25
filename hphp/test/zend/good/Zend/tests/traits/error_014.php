<?hh

trait foo {
    public function test() { return 3; }
}

class baz {
    final public function test() { return 4; }
}

class bar extends baz {
    use foo { test as public; }
}
<<__EntryPoint>> function main(): void {
$x = new bar;
var_dump($x->test());
}
