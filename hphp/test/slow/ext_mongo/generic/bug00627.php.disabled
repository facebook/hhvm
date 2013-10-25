<?php
require_once __DIR__."/../utils/server.inc";

$m = mongo_standalone();
$c = $m->selectCollection(dbname(), 'bug627');
$c->drop();

foreach (range(1,5) as $x) {
    $c->insert(array('x' => $x));
}

$group = array('$group' => array('_id' => 1, 'count' => array('$sum' => 1)));
$project = array('$project' => array('count' => 1));

$rs1 = $c->aggregate($group);
$rs2 = $c->aggregate(array($group));
$rs3 = $c->aggregate($group, $project);
$rs4 = $c->aggregate(array($group, $project));

var_dump($rs1 === $rs2);
var_dump($rs2 === $rs3);
var_dump($rs3 === $rs4);
var_dump($rs1);
