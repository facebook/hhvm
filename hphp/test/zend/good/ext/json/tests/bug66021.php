<?hh

class Foo {
    private $bar = 'baz';
}
<<__EntryPoint>> function main(): void {
echo json_encode(varray[varray[], new stdClass(), new Foo], JSON_PRETTY_PRINT), "\n";
}
