<?hh <<__EntryPoint>> function main(): void {
$string = "He had had to have had it";
$count = 0;
$newstring = str_replace_with_count("had", "foo", $string, inout $count);
print "$count changes were made.\n";
$count = "foo";
$newstring = str_replace_with_count("had", "foo", $string, inout $count);
print "$count changes were made.\n";
}
