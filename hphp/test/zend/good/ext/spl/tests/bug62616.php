<?hh <<__EntryPoint>> function main(): void {
$ai = new ArrayIterator(varray[0,1]);

var_dump($ai->count());

$ii = new IteratorIterator($ai);

var_dump($ii->count());
}
