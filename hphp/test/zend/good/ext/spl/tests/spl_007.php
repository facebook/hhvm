<?hh

class Foo {
    public function __call($name, $params) {
        echo "Called $name.\n";
        return true;
    }
}
<<__EntryPoint>> function main(): void {
$it = new ArrayIterator(varray[1, 2, 3]);

iterator_apply($it, varray[new Foo, "foobar"]);

echo "===DONE===\n";
}
