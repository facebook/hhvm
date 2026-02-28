<?hh
<<__EntryPoint>> function main(): void {
$a = vec[1,2,3];
$b = vec[];

try {
  $c = (int)($a) % (int)($b);
} catch (DivisionByZeroException $e) {
    echo $e->getMessage(), "\n";
}

echo "Done\n";
}
