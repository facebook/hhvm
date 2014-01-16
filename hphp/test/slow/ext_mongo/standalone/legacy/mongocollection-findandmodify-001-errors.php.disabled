<?php
require __DIR__."/../../utils/server.inc";

$m = mongo_standalone();
$col = $m->selectDB(dbname())->jobs;

$col->remove();

$col->insert(array(
     "name" => "Next promo",
     "inprogress" => false,
     "priority" => 0,
     "tasks" => array( "select product", "add inventory", "do placement"),
) );

$col->insert(array(
     "name" => "Biz report",
     "inprogress" => false,
     "priority" => 1,
     "tasks" => array( "run sales report", "email report" )
) );

$col->insert(array(
     "name" => "Biz report",
     "inprogress" => false,
     "priority" => 2,
     "tasks" => array( "run marketing report", "email report" )
    ),
    array("safe" => true)
);



try {
    $retval = $col->findAndModify(
         array("inprogress" => false, "name" => "Biz report"),
         array('$set' => array('$set' => array('inprogress' => true, "started" => new MongoDate()))),
         null,
         array(
            "sort" => array("priority" => -1),
            "new" => true,
        )
    );
} catch(MongoResultException $e) {
    echo $e->getCode(), " : ", $e->getMessage(), "\n";
    $res = $e->getDocument();
    var_dump($res["lastErrorObject"], $res["errmsg"], $res["ok"]);
}


try {
    $retval = $col->findAndModify(
         array("inprogress" => false, "name" => "Next promo"),
         array('$pop' => array("tasks" => -1)),
         array("tasks" => array('$pop' => array("stuff"))),
         array("new" => true)
    );
} catch(MongoResultException $e) {
    echo $e->getCode(), " : ", $e->getMessage(), "\n";
    var_dump($e->getDocument());
}


try {
    $col->findAndModify(
        null,
        array("asdf"),
        null,
        array("sort" => array("priority" => -1), "remove" => true)
    );
    $retval = $col->find();
    var_dump(iterator_to_array($retval));
    $col->remove();

} catch(MongoResultException $e) {
    echo $e->getCode(), " : ", $e->getMessage(), "\n";
    var_dump($e->getDocument());
}

try {
    $retval = $col->findAndModify(null);
    var_dump($retval);
} catch(MongoResultException $e) {
    echo $e->getCode(), " : ", $e->getMessage(), "\n";
    $res = $e->getDocument();
    var_dump($res["errmsg"], $res["ok"]);
}

?>