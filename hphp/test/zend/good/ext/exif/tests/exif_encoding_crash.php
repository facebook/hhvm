<?hh <<__EntryPoint>> function main(): void {
$infile = dirname(__FILE__).'/exif_encoding_crash.jpg';
$exif_data = exif_read_data($infile);
echo "*** no core dump ***\n";
echo "===DONE===\n";
}
