<?hh

interface test {
    public function bar():mixed;
}

class foo implements test {

    public function bar($arg = 2) :mixed{
        var_dump($arg);
    }
}
<<__EntryPoint>> function main(): void {
$foo = new foo;
$foo->bar();
}
