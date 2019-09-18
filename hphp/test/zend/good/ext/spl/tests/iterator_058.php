<?hh
class myIterator implements Iterator {

    function current() {}
    function next() {}
    function key() {}
    function valid() {}
    function rewind() {}

}
<<__EntryPoint>> function main(): void {
try {
    $it = new myIterator();
} catch (InvalidArgumentException $e) {
    echo 'InvalidArgumentException thrown';
}
echo 'no Exception thrown';
}
