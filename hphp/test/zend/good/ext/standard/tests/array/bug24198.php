<?hh <<__EntryPoint>> function main(): void {
$c = darray['a' => 'aa','b' => 'bb'];

var_dump(array_merge_recursive($c, $c));
}
