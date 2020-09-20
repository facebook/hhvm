<?hh <<__EntryPoint>> function main(): void {
$infile = dirname(__FILE__).'/bug34704.jpg';
var_dump(exif_read_data($infile));
echo "===DONE===\n";
}
