<?php
require_once __DIR__."/../utils/server.inc";

$m = mongo_standalone();
$c = $m->selectDb(dbname())->bug602;
$c->remove();
$c->insert( array( 'test' => 'one' ) );
$c->insert( array( 'test' => 'two' ) );
$c->insert( array( 'test' => 'three' ) );
$c->insert( array( 'test' => 'four' ) );
$c->insert( array( 'test' => 'five' ) );
$c->insert( array( 'test' => 'six' ) );
$c->insert( array( 'test' => 'seven' ) );
$cursor = $c->find()->skip(3)->limit(2);
var_dump($cursor->info());
$cursor->getNext();
var_dump($cursor->info());
?>