<?hh

trait foo {
    public function abc() {
    }
}

interface baz {
    public function abc();
}

class bar implements baz {
    use foo;

}
<<__EntryPoint>> function main(): void {
new bar;
print "OK\n";
}
