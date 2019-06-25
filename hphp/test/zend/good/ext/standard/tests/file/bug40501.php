<?hh <<__EntryPoint>> function main(): void {
$file = dirname(__FILE__).'/bug40501.csv';

$h = fopen($file, 'r');
$data = fgetcsv($h, 0, ',', '"', '"');
fclose($h);

var_dump($data);
}
