<?hh <<__EntryPoint>> function main(): void {
$file = __DIR__ ."/data.txt";
file_put_contents($file, "foobar");

$s = new SplFileObject( $file );
echo $s->getSize();
error_reporting(0);
$file = __DIR__ ."/data.txt";
unlink($file);
}
