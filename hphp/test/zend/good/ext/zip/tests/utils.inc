<?hh
/* $Id$ */
function dump_entries_name($z) {
	for($i=0; $i<$z->numFiles; $i++) {
	    $sb = $z->statIndex($i);
	    echo $i . ' ' . $sb['name'] . "\n";
	}
}
/* recursively remove a directoryy */
function rmdir_rf($dir) {
	$handle = opendir($dir);
	if ($handle) {
		$item = readdir($handle);
		while (false !== $item) {
			if ($item != "." && $item != "..") {
				if (is_dir($dir . '/' . $item)) {
					rmdir_rf($dir . '/' . $item);
				} else {
					unlink($dir . '/' . $item);
				}
			}
			$item = readdir($handle);
		}
		closedir($handle);
		rmdir($dir);
	}
}
