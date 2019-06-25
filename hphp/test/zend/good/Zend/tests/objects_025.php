<?hh

class foo {
    public function __call($a, $b) {
        print "non-static - ok\n";
    }
}
<<__EntryPoint>> function main(): void {
$a = new foo;
$a->foooo();

$b = 'aaaaa1';
$a->$b();

$b = '  ';
$a->$b();

$b = str_repeat('a', 10000);
$a->$b();

$b = NULL;
$a->$b();
}
