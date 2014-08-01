<?php
/* This test might get very long depending on the mashine it's running on. Therefore
adding an explicit skip, remove it to run this test. */
set_time_limit(0);

$base_path = dirname(__FILE__);

/* Either we ship a file with 1000 entries which would be >12M big,
	or create it dynamically. */
$zip = new ZipArchive;
$r = $zip->open("$base_path/51353.zip", ZIPARCHIVE::CREATE | ZIPARCHIVE::OVERWRITE);
if ($r) {
	for ($i = 0; $i < 1000; $i++) {
		$zip->addFromString("$i.txt", '1');
	}
	$zip->close();
} else {
	die("failed");
}

$zip = new ZipArchive;
$r = $zip->open("$base_path/51353.zip");
if ($r) {
	$zip->extractTo("$base_path/51353_unpack");
	$zip->close();

	$a = glob("$base_path/51353_unpack/*.txt");
	echo count($a) . "\n";
} else {
	die("failed");
}

echo "OK";?>
<?php
$base_path = dirname(__FILE__);

unlink("$base_path/51353.zip");

$a = glob("$base_path/51353_unpack/*.txt");
foreach($a as $f) {
	unlink($f);
}
rmdir("$base_path/51353_unpack");