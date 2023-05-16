<?hh
<<__EntryPoint>> function main(): void {
chdir(sys_get_temp_dir());
$fp = fopen('SplFileObject__fgetcsv8.csv', 'w+');
fwrite($fp, '"aaa","b""bb","ccc"');
fclose($fp);

$fo = new SplFileObject('SplFileObject__fgetcsv8.csv');
var_dump($fo->fgetcsv(',', '"', 'invalid'));

unlink('SplFileObject__fgetcsv8.csv');
}
