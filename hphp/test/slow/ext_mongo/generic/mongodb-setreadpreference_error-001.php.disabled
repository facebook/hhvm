<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

$modes = array("blaat", 42, true, 3.14);

foreach ($modes as $mode) {
    $m = new_mongo_standalone(null, true, true, array('readPreference' => MongoClient::RP_PRIMARY_PREFERRED));
    $db = $m->phpunit;
    $db->setReadPreference($mode);
    $rp = $db->getReadPreference();
    echo $rp["type"], "\n";
}
?>