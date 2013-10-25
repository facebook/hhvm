<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

$tagsets = array(
    /* no tagsets */
    array(),
    /* one tag set */
    array( array( 'dc' => 'east' ) ),
    array( array( 'dc' => 'east', 'use' => 'reporting' ) ),
    array( array() ),
    /* two tag sets */
    array( array( 'dc' => 'east', 'use' => 'reporting' ), array( 'dc' => 'west' ) ),
    /* two tag sets + empty one */
    array( array( 'dc' => 'east', 'use' => 'reporting' ), array( 'dc' => 'west' ), array() ),
);

foreach ($tagsets as $tagset) {
    $m = new_mongo_standalone();
    $c = $m->phpunit->test;
    $c->setReadPreference(Mongo::RP_SECONDARY, $tagset);
    $rp = $c->getReadPreference();
    var_dump($rp);

    echo "---\n";
}
?>