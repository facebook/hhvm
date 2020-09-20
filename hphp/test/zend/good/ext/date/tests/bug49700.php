<?hh <<__EntryPoint>> function main(): void {
gc_enable();
$objs = darray[];
$objs[1] = new DateTime();
gc_collect_cycles();
unset($objs);
echo "OK\n";
}
