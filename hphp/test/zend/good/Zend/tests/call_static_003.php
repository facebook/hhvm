<?hh

class Foo {
    public function __call($a, $b) {
        print "nonstatic\n";
        var_dump($a);
    }
    public function test() {
        $this->fOoBaR();
        self::foOBAr();
        $this::fOOBAr();
    }
}
<<__EntryPoint>> function main(): void {
$a = new Foo;
$a->test();
}
