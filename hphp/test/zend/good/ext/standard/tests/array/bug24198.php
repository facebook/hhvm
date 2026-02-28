<?hh <<__EntryPoint>> function main(): void {
$c = dict['a' => 'aa','b' => 'bb'];

var_dump(array_merge_recursive($c, $c));
}
