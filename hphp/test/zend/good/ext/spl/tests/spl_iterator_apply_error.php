<?hh

class MyArrayIterator extends ArrayIterator {
    public function rewind() :mixed{
        throw new Exception('Make the iterator break');
    }
}

function test() :mixed{}
<<__EntryPoint>> function main(): void {
$it = new MyArrayIterator(vec[1, 21, 22]);

try {
    $res = iterator_apply($it, test<>);
} catch (Exception $e) {
    echo $e->getMessage();
}
}
