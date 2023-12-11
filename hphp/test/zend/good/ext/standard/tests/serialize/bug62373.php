<?hh
class A {}
class B {}
<<__EntryPoint>> function main(): void {
$size_of_ce = (((int)(log((float)PHP_INT_MAX) / log(2.0)) + 1 == 32 ? 368: 680) + 15) & ~15;
$dummy = vec[];
$b = new B();
$period = $size_of_ce << 5;
for ($i = 0; $i < $period * 3; $i++) {
    $a = new A();
    $s = unserialize(serialize(vec[$b, $a]));
    if ($s[0] === $s[1]) {
        echo "OOPS\n";
        break;
    }
    $dummy[] = $a;
}

echo "OK\n";
}
