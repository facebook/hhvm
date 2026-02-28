<?hh
<<__EntryPoint>> function main(): void {
$file = sys_get_temp_dir().'/'.'data.txt';
file_put_contents($file, "foobar");

$s = new SplFileObject( $file );
echo $s->getSize();

unlink($file);
}
