<?hh

/* Testing fputcsv() to write to a file when default enclosure value and delimiter
   of two chars is provided */
<<__EntryPoint>> function main(): void {
echo "*** Testing fputcsv() : with default enclosure & delimiter of two chars ***\n";

$fo = new SplFileObject(__DIR__ . '/SplFileObject_fputcsv_variation13.csv', 'w');

var_dump($fo->fputcsv(varray['water', 'fruit'], ',,', '"'));

unset($fo);

echo "Done\n";
error_reporting(0);
$file = __DIR__ . '/SplFileObject_fputcsv_variation13.csv';
unlink($file);
}
