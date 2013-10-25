<?php
require_once __DIR__."/../utils/server.inc";
    $m = mongo_standalone("phpunit");
    $db = $m->selectDB("phpunit");
    $db->dropCollection("fs.files");
    $db->dropCollection("fs.chunks");

    $gridfs = $db->getGridFS();

    $tempFileName = tempnam(sys_get_temp_dir(), "gridfs-delete");
    $tempFileData = '1234567890';
    file_put_contents($tempFileName, $tempFileData);

    $ids = array(
        "file0",
        452,
        true,
        new MongoID(),
        array( 'a', 'b' => 5 ),
    );

    foreach ($ids as $id) {
        $gridfs->storeFile($tempFileName, array('_id' => $id));
        $file = $gridfs->get($id);

        echo 'File exists with ID: ';
        var_dump($file->file['_id']);
        echo "\n";
    }

    unlink($tempFileName);
?>