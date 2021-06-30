<?hh
<<__EntryPoint>> function main(): void {
$dirname = dirname(__FILE__) . '/';
$zip = new ZipArchive;
if (!$zip->open($dirname . 'test.zip')) {
	exit('failed');
}

if ($zip->status == ZipArchive::ER_OK) {
	$zip->close();
	echo "ok\n";
} else {
	echo "failed\n";
}
}
