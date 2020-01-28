<?hh


<<__EntryPoint>>
function main_array_iterator_seek_bounds() {
$array = varray[1];
$i = new ArrayIterator($array);

try {
    $i->seek(-1);
} catch (Exception $e) {
    $class = get_class($e);
    echo "{$class}: {$e->getMessage()}\n";
}
try {
    $i->seek(1);
} catch (Exception $e) {
    $class = get_class($e);
    echo "{$class}: {$e->getMessage()}\n";
}
}
