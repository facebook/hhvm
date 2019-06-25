<?hh

class foo {
    function __unset($a) {
        print "unset\n";
        var_dump($a);
    }

    public function __call($a, $b) {
        print "call\n";
        var_dump($a);
    }
    function __clone() {
        print "clone\n";
    }

    public function __tostring() {
        return 'foo';
    }
}

<<__EntryPoint>> function main(): void {
$a = new foo;

$a->sdfdsa();

clone $a;

var_dump((string)$a);

unset($a->a);
}
