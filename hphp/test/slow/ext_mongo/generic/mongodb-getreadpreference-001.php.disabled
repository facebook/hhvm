<?php require_once __DIR__."/../utils/server.inc"; ?>
<?php

$baseString = sprintf("mongodb://%s:%d/%s?readPreference=", standalone_hostname(), standalone_port(), dbname());

$modes = array(
    'primary',
    'secondary',
);

$tagParams = array(
    '',
    '&readPreferenceTags=dc:west',
    '&readPreferenceTags=dc:west,use:reporting',
    '&readPreferenceTags=',
    '&readPreferenceTags=dc:west,use:reporting&readPreferenceTags=dc:east',
);

foreach ($modes as $mode) {
    foreach ($tagParams as $tagParam) {
        $m = new MongoClient($baseString . $mode . $tagParam, array('connect' => false));
        $rp = $m->phpunit->getReadPreference();
        var_dump($rp);
        echo "---\n";
    }
}
?>