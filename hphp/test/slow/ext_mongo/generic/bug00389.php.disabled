<?php
require_once __DIR__."/../utils/server.inc";
$m = mongo_standalone();

/* Ensure the collection actually exists. If not, the oplogReplay flag will not
 * cause an "no ts field in query" error on the server.
 */
$m->phpunit->createCollection('bug00389');
$c = $m->phpunit->bug00389;

/* Tailable */
try {
	$cursor = $c->find()->tailable();
	foreach( $cursor as $foo ) { }
} catch ( MongoCursorException $e ) {
	echo $e->getMessage(), "\n";
}

/* Slave okay */
$cursor = $c->find()->slaveOkay();
foreach( $cursor as $foo ) { }

/* Immortal */
$cursor = $c->find()->immortal();
foreach( $cursor as $foo ) { }

/* Await data */
$cursor = $c->find()->awaitData();
foreach( $cursor as $foo ) { }

/* Partial */
$cursor = $c->find()->partial();
foreach( $cursor as $foo ) { }

/* with setFlag() */
for ( $i = 1; $i < 11; $i++ )
{
	echo "Setting flag #", $i, "\n";
	try {
		$cursor = $c->find()->setFlag( $i );
		foreach( $cursor as $foo ) { }
	} catch ( MongoCursorException $e ) {
		echo $e->getMessage(), "\n";
	}
}
?>