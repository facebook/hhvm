<?hh
<<__EntryPoint>> function main(): void {
$it = new ArrayIterator(varray[]);

$lit = new LimitIterator($it, 0, 5);

foreach ($lit as $v) {
    echo $v;
}
echo "===DONE===\n";
}
