<?hh
<<__EntryPoint>> function main(): void {
$ini = parse_ini_string('ini="ini;raw"', false, INI_SCANNER_RAW);
var_dump($ini['ini']);
$ini = parse_ini_string('ini="ini;raw', false, INI_SCANNER_RAW);
var_dump($ini['ini']);
$ini = parse_ini_string('ini=ini;raw', false, INI_SCANNER_RAW);
var_dump($ini['ini']);
$ini = parse_ini_string('ini=ini"raw', false, INI_SCANNER_RAW);
var_dump($ini['ini']);
$ini = parse_ini_string("ini=\r\niniraw", false, INI_SCANNER_RAW);
var_dump($ini['ini']);
}
