<?hh
<<__EntryPoint>> function main(): void {
$s = shape('a' => 123, 'b' => 456);

var_dump(serialize($s));

var_dump(unserialize(serialize($s)));
}
