<?hh <<__EntryPoint>> function main(): void {
$fp = fopen(dirname(__FILE__).'/test3.csv', 'r');
var_dump(fgetcsv($fp, 4096));
}
