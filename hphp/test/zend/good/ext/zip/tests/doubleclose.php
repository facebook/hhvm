<?hh
<<__EntryPoint>> function main(): void {
echo "Procedural\n";
$zip = zip_open(dirname(__FILE__) . '/test.zip');
if (!is_resource($zip)) {
	exit("Failure");
	}
var_dump(zip_close($zip));
var_dump(zip_close($zip));

echo "Object\n";
$zip = new ZipArchive();
if (!$zip->open(dirname(__FILE__) . '/test.zip')) {
	exit('Failure');
}
if ($zip->status == ZipArchive::ER_OK) {
	var_dump($zip->close());
	var_dump($zip->close());
} else {
	exit("Failure");
}
echo "Done";
}
