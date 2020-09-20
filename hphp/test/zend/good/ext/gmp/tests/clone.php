<?hh
<<__EntryPoint>> function main(): void {
$a = gmp_init(3);
$b = clone $a;
gmp_clrbit(inout $a, 0);
var_dump($a, $b); // $b should be unaffected
}
