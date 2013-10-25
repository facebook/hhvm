<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

function myerror($errno, $errstr) {
    var_dump($errno, $errstr);
}
set_error_handler("myerror", E_RECOVERABLE_ERROR);

$tagsets = array(
    42,
    "string",
    array( 42 ),
    array( array( 42 ) ),
    array( array( 'bar' => 'foo', 42 ) ),
    array( array( 42, 'bar' => 'foo' ) ),
    array( array( 'bar' => 'foo' ), array( 42 ) ),
    array( array( 'foo' ), array( 42 ) ),
);

foreach ($tagsets as $tagset) {
    $m = new_mongo_standalone();
    $db = $m->phpunit;
    $db->setReadPreference(MongoClient::RP_SECONDARY, $tagset);
    $rp = $db->getReadPreference();
    var_dump($rp);

    echo "---\n";
}
?>
==DONE==
<?php exit(0); ?>