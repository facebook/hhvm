<?hh
class myIterator implements Iterator {

    function current() :mixed{}
    function next() :mixed{}
    function key() :mixed{}
    function valid() :mixed{}
    function rewind() :mixed{}

}
<<__EntryPoint>> function main(): void {
try {
    $it = new myIterator();
} catch (InvalidArgumentException $e) {
    echo 'InvalidArgumentException thrown';
}
echo 'no Exception thrown';
}
