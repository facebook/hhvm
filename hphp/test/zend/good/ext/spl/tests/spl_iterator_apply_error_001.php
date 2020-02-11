<?hh

function test() {
    throw new Exception('Broken callback');
}
<<__EntryPoint>> function main(): void {
$it = new RecursiveArrayIterator(varray[1, 21, 22]);

try {
    iterator_apply($it, 'test');
} catch (Exception $e) {
    echo $e->getMessage();
}
}
