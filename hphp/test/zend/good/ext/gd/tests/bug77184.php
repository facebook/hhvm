<?hh <<__EntryPoint>> function main(): void {
$infile = __DIR__.'/bug77184.DJI_0245_tiny.jpg';
$tags = exif_read_data($infile);
echo $tags['GPSLatitude'][2], PHP_EOL;
echo $tags['GPSLongitude'][2], PHP_EOL;
}
