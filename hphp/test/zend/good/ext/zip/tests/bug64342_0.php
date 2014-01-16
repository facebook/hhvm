<?php

$zip = new ZipArchive;
$res = $zip->open(dirname(__FILE__) . '/bug64342.zip', ZipArchive::CREATE);
if ($res === TRUE) {
	$f = md5(uniqid()) . '.txt';
	echo "$f\n";
	$res = $zip->addFile($f);
	if (true == $res) {
		echo "add ok\n";
	} else {
		echo "add failed\n";
	}
	$res = $zip->close();
	if (true == $res) {
		echo "close ok\n";
	} else {
		echo "close failed\n";
	}
} else {
	echo "open failed\n";
}


?>
DONE
<?php

@unlink(dirname(__FILE__) . '/bug64342.zip');