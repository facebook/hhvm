<?hh
<<__EntryPoint>> function main(): void {
gc_enable();
$gmp = gmp_init('10');
gc_collect_cycles();

echo "===DONE===\n";
}
