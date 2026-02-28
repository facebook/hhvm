<?hh
<<__EntryPoint>> function main(): void {
$fo = new SplFileObject(sys_get_temp_dir().'/'.'SplFileObject_fputcsv1.csv', 'w');

$data = vec[1, 2, 'foo', 'haha', 1.3, null];

$fo->fputcsv($data);

var_dump($data);

unlink(sys_get_temp_dir().'/'.'SplFileObject_fputcsv1.csv');
}
