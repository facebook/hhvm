<?hh

class MyArrayIterator extends ArrayIterator {
    public function current() :mixed{
        throw new Exception('Make the iterator break');
    }
}
<<__EntryPoint>> function main(): void {
$it = new MyArrayIterator(vec[4, 6, 2]);

try {
    // get keys
    $ar = iterator_to_array($it);
} catch (Exception $e) {
    echo $e->getMessage() . PHP_EOL;
}

try {
    // get values
    $ar = iterator_to_array($it, false);
} catch (Exception $e) {
    echo $e->getMessage() . PHP_EOL;
}
}
