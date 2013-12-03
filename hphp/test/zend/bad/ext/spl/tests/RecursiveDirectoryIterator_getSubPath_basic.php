<?php
$depth0 = "depth0";
$depth1 = 'depth1';
$depth2 = 'depth2';
$targetDir = __DIR__ . DIRECTORY_SEPARATOR . $depth0 . DIRECTORY_SEPARATOR . $depth1 . DIRECTORY_SEPARATOR . $depth2;
mkdir($targetDir, 0777, true);
touch($targetDir . DIRECTORY_SEPARATOR . 'getSubPath_test.tmp');
$iterator = new RecursiveDirectoryIterator(__DIR__ . DIRECTORY_SEPARATOR . $depth0);
$it = new RecursiveIteratorIterator($iterator);

$list = [];
while($it->valid()) {
  $list[] = $it->getSubPath();
  $it->next();
}
asort($list);
foreach ($list as $item) {
	echo $item . "\n";
}
?>
<?php
function rrmdir($dir) {
    foreach(glob($dir . '/*') as $file) {
        if(is_dir($file)) {
            rrmdir($file);
        } else {
            unlink($file);
        }
    }

    rmdir($dir);
}

$targetDir = __DIR__.DIRECTORY_SEPARATOR . "depth0";
rrmdir($targetDir);
?>
