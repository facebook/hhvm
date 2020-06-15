<?hh

/* Testing fputcsv() to write to a file when default enclosure value and delimiter
   of two chars is provided */
<<__EntryPoint>> function main(): void {
echo "*** Testing fputcsv() : with default enclosure & delimiter of two chars ***\n";

$file = __SystemLib\hphp_test_tmppath('SplFileObject_fputcsv_variation13.csv');
$fo = new SplFileObject($file, 'w');

var_dump($fo->fputcsv(varray['water', 'fruit'], ',,', '"'));

unset($fo);

echo "Done\n";

unlink($file);
}
