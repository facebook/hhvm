<?hh
<<__EntryPoint>> function main(): void {
$file = __SystemLib\hphp_test_tmppath('fgetcsv-test.csv');

$data = varray[1, 2, 'foo', 'haha', 1.3, null];

$fp = fopen($file, 'w');

fputcsv($fp, $data);

var_dump($data);

@unlink($file);
}
