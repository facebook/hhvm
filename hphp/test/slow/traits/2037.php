<?hh

trait foo {
    public function abc() :mixed{
    }
}

interface baz {
    public function abc():mixed;
}

class bar implements baz {
    use foo;

}
<<__EntryPoint>> function main(): void {
new bar;
print "OK\n";
}
