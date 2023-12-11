<?hh

/* Testing fputcsv() to write to a file when default enclosure value and delimiter
   of two chars is provided and file is opened in read only mode */
<<__EntryPoint>> function main(): void {
echo "*** Testing fputcsv() : with enclosure & delimiter of two chars and file opened in read mode ***\n";

$fo = new SplFileObject(sys_get_temp_dir().'/'.'SplFileObject_fputcsv_variation14.csv', 'w');

var_dump($fo->fputcsv(vec['water', 'fruit'], ',,', '""'));

unset($fo);

echo "Done\n";

unlink(sys_get_temp_dir().'/'.'SplFileObject_fputcsv_variation14.csv');
}
