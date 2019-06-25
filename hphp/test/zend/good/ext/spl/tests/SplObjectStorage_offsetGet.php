<?hh
<<__EntryPoint>> function main(): void {
$s = new SplObjectStorage();
$o1 = new stdClass();
$s[$o1] = 'some_value';

echo $s->offsetGet($o1);
}
