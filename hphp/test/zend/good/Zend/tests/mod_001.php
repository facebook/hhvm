<?hh
<<__EntryPoint>> function main(): void {
$a = varray[1,2,3];
$b = array();

try {
  $c = $a % $b;
} catch (DivisionByZeroException $e) {
    echo $e->getMessage(), "\n";
}

echo "Done\n";
}
