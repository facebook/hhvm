<?hh <<__EntryPoint>> function main(): void {
gc_enable();
$objs = dict[];
$objs[1] = new DateTime();
gc_collect_cycles();
$objs = null;
echo "OK\n";
}
