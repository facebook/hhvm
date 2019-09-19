<?hh
/**
 * From Docs: Construct a new array iterator from anything that has a hash table.
 * NULL, NOTHING is not a hash table ;)
 */
class myArrayIterator extends ArrayIterator {
}
<<__EntryPoint>> function main(): void {
try {
    $it = new myArrayIterator();
} catch (InvalidArgumentException $e) {
    echo 'InvalidArgumentException thrown';
}
echo 'no Exception thrown';
}
