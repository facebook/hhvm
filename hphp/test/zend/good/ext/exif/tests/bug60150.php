<?hh <<__EntryPoint>> function main(): void {
$infile = dirname(__FILE__).'/bug60150.jpg';
var_dump(exif_read_data($infile));
echo "===DONE===\n";
}
