<?php
require_once __DIR__."/../utils/server.inc";
$a = mongo_standalone();
$d = $a->selectDb(dbname());

$d->listcol->drop();
$d->listcol->insert(array('_id' => 'test'));

echo "without flag\n";
$collections = $d->getCollectionNames();
foreach( $collections as $col )
{
	if ($col == 'system.indexes') {
		echo $col, "\n";
	}
}

echo "with flag\n";
$collections = $d->getCollectionNames(true);
foreach( $collections as $col )
{
	if ($col == 'system.indexes') {
		echo $col, "\n";
	}
}
?>