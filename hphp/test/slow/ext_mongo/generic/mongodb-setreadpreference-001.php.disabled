<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

$modes = array(
    Mongo::RP_PRIMARY,
    Mongo::RP_PRIMARY_PREFERRED,
    Mongo::RP_SECONDARY,
    Mongo::RP_SECONDARY_PREFERRED,
    Mongo::RP_NEAREST
);

foreach (array_values($modes) as $mode) {
    $m = new_mongo_standalone(null, true, true, array('readPreference' => $mode));
    $db = $m->phpunit;
    echo $mode, "\n\n";
    foreach (array_values($modes) as $newMode) {
        $db->setReadPreference($newMode);
        $rp = $db->getReadPreference();
        echo $rp["type"], "\n";
    }
    echo "---\n";
}
?>