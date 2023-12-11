<?hh <<__EntryPoint>> function main(): void {
$single_barline = "\360\235\204\200";
$array = vec[$single_barline];
print bin2hex($single_barline) . "\n";
// print $single_barline . "\n\n";
$json = json_encode($array);
print $json . "\n\n";
$json_decoded = json_decode($json, true);
// print $json_decoded[0] . "\n";
print bin2hex($json_decoded[0]) . "\n";
print "END\n";
}
