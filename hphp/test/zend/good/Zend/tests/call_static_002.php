<?hh

class Foo {
    public function __call($a, $b) {
        print "nonstatic\n";
        var_dump($a);
    }
}
<<__EntryPoint>> function main(): void {
$a = new Foo;
call_user_func(varray[$a, 'aAa']);
}
