<?hh
<<__EntryPoint>> function main(): void {
$file = sys_get_temp_dir().'/'.'fgetcsv-test.csv';

$data = vec[1, 2, 'foo', 'haha', 1.3, null];

$fp = fopen($file, 'w');

fputcsv($fp, $data);

var_dump($data);

unlink($file);
}
