<?hh
<<__EntryPoint>> function main(): void {
$ini_location = dirname(__FILE__) . '/bug48768.tmp';

// Build ini data
$ini_data = <<< EOT
equal = "="

EOT;

// Save ini data to file
file_put_contents($ini_location, $ini_data);

var_dump(parse_ini_file($ini_location, false, INI_SCANNER_RAW));
var_dump(parse_ini_file($ini_location, false, INI_SCANNER_NORMAL));
error_reporting(0);
@unlink(dirname(__FILE__) . '/bug48768.tmp');
}
