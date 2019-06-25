<?hh

class a {
    public function __call($a, $b) {
        print "__call: ". $a ."\n";
    }
    public function baz() {
        self::Bar();
    }
}

<<__EntryPoint>> function main(): void {
$a = new a;

$b = 'Test';
$a->$b();

$a->baz();
}
