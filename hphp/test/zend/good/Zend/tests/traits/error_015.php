<?hh

trait foo {
    public function test() { return 3; }
}

trait baz {
    public function test() { return 4; }
}

class bar {
    use foo, baz {
        baz::test as zzz;
    }
}
<<__EntryPoint>> function main(): void {
$x = new bar;
var_dump($x->test());
}
