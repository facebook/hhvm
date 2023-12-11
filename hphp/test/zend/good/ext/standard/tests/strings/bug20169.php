<?hh <<__EntryPoint>> function main(): void {
@set_time_limit(5);
$delimiter = "|";

echo "delimiter: $delimiter\n";
implode($delimiter, vec["foo", "bar"]);
echo "delimiter: $delimiter\n";
}
