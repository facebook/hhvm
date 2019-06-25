<?hh

interface test {
    public function bar();
}

class foo implements test {

    public function bar($arg = 2) {
        var_dump($arg);
    }
}
<<__EntryPoint>> function main(): void {
$foo = new foo;
$foo->bar();
}
