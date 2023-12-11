<?hh
<<__EntryPoint>> function main(): void {
chdir(sys_get_temp_dir());
$fp = fopen('SplFileObject__fgetcsv3.csv', 'w+');
fputcsv($fp, vec[
    'field1',
    'field2',
    'field3',
    5
], '|');
fclose($fp);

$fo = new SplFileObject('SplFileObject__fgetcsv3.csv');
var_dump($fo->fgetcsv('invalid'));

unlink('SplFileObject__fgetcsv3.csv');
}
