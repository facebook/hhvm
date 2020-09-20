<?hh
<<__EntryPoint>> function main(): void {
$file = __SystemLib\hphp_test_tmppath('data.txt');
file_put_contents($file, "foobar");

$s = new SplFileObject( $file );
echo $s->getSize();

unlink($file);
}
