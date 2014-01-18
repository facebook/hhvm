<?php
require_once __DIR__."/../../utils/server.inc";
$mongo = new_mongo_standalone();
$gridfs = $mongo->files->getGridFS();
$i = 0;
foreach(glob(dirname(__FILE__) . "/*") as $file) {
    $gridfs->put($file);
    if ($i++ > 10) {
        break;
    }
}

$file = $mongo->files->getGridFS()->find()->sort(array('length' => -1))->limit(1)->getNext();

$attempts = 10;
while ($attempts--) {
	$mongo->files->getGridFS()->find()->sort(array('length' => -1))->limit(1)->getNext()->write('./test.bin');
}
@unlink("./test.bin");

echo "No memory leaks should be reported\n";
?>