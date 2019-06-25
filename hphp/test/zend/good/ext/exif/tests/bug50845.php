<?hh <<__EntryPoint>> function main(): void {
$infile = dirname(__FILE__).'/bug50845.jpg';
var_dump(exif_read_data($infile));
}
