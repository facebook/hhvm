<?php
require_once __DIR__."/../utils/server.inc";

$m = mongo_standalone();
$c = $m->selectDB("phpunit")->selectCollection("article");
$c->drop();
$data = array (
    'title' => 'this is my title',
    'author' => 'bob',
    'posted' => new MongoDate,
    'pageViews' => 5,
    'tags' => 
    array (
      0 => 'fun',
      1 => 'good',
      2 => 'fun',
    ),
    'comments' => 
    array (
      0 => 
      array (
        'author' => 'joe',
        'text' => 'this is cool',
      ),
      1 => 
      array (
        'author' => 'sam',
        'text' => 'this is bad',
      ),
    ),
    'other' => 
    array (
      'foo' => 5,
    ),
);
$d = $c->insert($data, array("safe" => true));

$ops = array(
    array(
        '$project' => array(
            "author" => 1,
            "tags"   => 1,
        )
    ),
    array('$unwind' => '$tags'),
    array(
        '$group' => array(
            "_id" => array("tags" => '$tags'),
            "authors" => array('$addToSet' => '$author'),
        )
    )
);


$alone = $c->aggregate($ops);
$multiple = $c->aggregate(current($ops), next($ops), next($ops));
var_dump($alone == $multiple, $alone, $multiple);
$c->drop();

$c->aggregate();
$c->aggregate("string");
$c->aggregate($ops, "string");
$retval = $c->aggregate((object)$ops);
?>
===DONE===
<?php exit(0); ?>