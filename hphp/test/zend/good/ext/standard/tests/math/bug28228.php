<?hh <<__EntryPoint>> function main(): void {
echo number_format(1234.5678, 4, '', '') . "\n";
echo number_format(1234.5678, 4, NULL, ',') . "\n";
echo number_format(1234.5678, 4, 0, ',') . "\n";
echo number_format(1234.5678, 4);
}
