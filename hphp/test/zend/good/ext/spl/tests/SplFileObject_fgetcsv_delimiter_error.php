<?hh <<__EntryPoint>> function main(): void {
$fp = fopen('SplFileObject__fgetcsv3.csv', 'w+');
fputcsv($fp, varray[
    'field1',
    'field2',
    'field3',
    5
], '|');
fclose($fp);

$fo = new SplFileObject('SplFileObject__fgetcsv3.csv');
var_dump($fo->fgetcsv('invalid'));
error_reporting(0);
unlink('SplFileObject__fgetcsv3.csv');
}
