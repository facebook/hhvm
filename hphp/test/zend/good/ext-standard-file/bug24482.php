<?php
$globdirs = glob(__DIR__."/../../../sample_dir/(", GLOB_ONLYDIR);

$dirs = array();
$dh = opendir(__DIR__."/../../../sample_dir/");
while (is_string($file = readdir($dh))) {
	if ($file[0] === ".") continue;
	if (!is_dir($file)) continue;
	$dirs[] = $file;
}
closedir($dh);

if (count($dirs) != count($globdirs)) {
	echo "Directory count mismatch\n";
	
	echo "glob found:\n";
	sort($globdirs);	
	var_dump($globdirs);
	
	echo "opendir/readdir/isdir found:\n";
	sort($dirs);	
	var_dump($dirs);
} else {
	echo "OK\n";
}
?>