<?hh <<__EntryPoint>> function main(): void {
$a = stat(__FILE__);
is_link(__FILE__);
$b = stat(__FILE__);
print_r(array_diff($a, $b));
}
