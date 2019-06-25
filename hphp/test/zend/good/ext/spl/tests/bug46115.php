<?hh <<__EntryPoint>> function main(): void {
$h = new RecursiveArrayIterator(array());
$x = new reflectionmethod('RecursiveArrayIterator', 'asort');
$z = $x->invoke($h);
echo "DONE";
}
