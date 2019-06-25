<?hh <<__EntryPoint>> function main(): void {
$fp = fopen('SplFileObject__fgetcsv8.csv', 'w+');
fwrite($fp, '"aaa","b""bb","ccc"');
fclose($fp);

$fo = new SplFileObject('SplFileObject__fgetcsv8.csv');
var_dump($fo->fgetcsv(',', '"', 'invalid'));
error_reporting(0);
unlink('SplFileObject__fgetcsv8.csv');
}
