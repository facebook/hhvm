<?hh
<<__EntryPoint>> function main(): void {
$fo = new SplFileObject(__SystemLib\hphp_test_tmppath('SplFileObject_fputcsv1.csv'), 'w');

$data = varray[1, 2, 'foo', 'haha', varray[4, 5, 6], 1.3, null];

$fo->fputcsv($data);

var_dump($data);

unlink(__SystemLib\hphp_test_tmppath('SplFileObject_fputcsv1.csv'));
}
