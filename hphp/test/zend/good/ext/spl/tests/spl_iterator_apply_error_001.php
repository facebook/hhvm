<?hh

function test() {
    throw new Exception('Broken callback');
}
<<__EntryPoint>> function main() {
$it = new RecursiveArrayIterator(array(1, 21, 22));

try {
    iterator_apply($it, 'test');
} catch (Exception $e) {
    echo $e->getMessage();
}
}
