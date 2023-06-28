<?hh

interface test {
    public function bar():mixed;
}

class foo implements test {

    public function bar($foo = NULL) :mixed{
        echo "foo\n";
    }
}
<<__EntryPoint>> function main(): void {
error_reporting(4095);

$foo = new foo;
$foo->bar();
}
