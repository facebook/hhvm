<?hh <<__EntryPoint>> function main(): void {
$h = new RecursiveArrayIterator(varray[]);
$x = new reflectionmethod('RecursiveArrayIterator', 'asort');
$z = $x->invoke($h);
echo "DONE";
}
