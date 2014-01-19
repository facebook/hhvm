<?php
require_once __DIR__."/../utils/server.inc";

$host = MongoShellServer::getStandaloneInfo();


function log_query($server, $query, $cursor_options) {
    var_dump($query);
}

$ctx = stream_context_create(
    array(
        "mongodb" => array(
            "log_query" => "log_query",
        )
    )
);

$mc = new MongoClient($host, array("journal" => true), array("context" => $ctx));

echo "journal enabled by default\n";
$doc = array("doc" => "ument");
$mc->test->bug572->insert($doc);
$mc->test->bug572->update(array("_id" => $doc["_id"]), array("updated" => "doc"));
$mc->test->bug572->remove(array("_id" => $doc["_id"]));

echo "Setting it to false, per-query\n";
$doc = array("doc" => "ument");
$mc->test->bug572->insert($doc, array("j" => false));
$mc->test->bug572->update(array("_id" => $doc["_id"]), array("updated" => "doc"), array("j" => false));
$mc->test->bug572->remove(array("_id" => $doc["_id"]), array("j" => false));

echo "Setting it to false, per-query, and w=0 to force no-gle\n";
$doc = array("doc" => "ument");
$mc->test->bug572->insert($doc, array("j" => false, "w" => 0));
$mc->test->bug572->update(array("_id" => $doc["_id"]), array("updated" => "doc"), array("j" => false, "w" => 0));
$mc->test->bug572->remove(array("_id" => $doc["_id"]), array("j" => false, "w" => 0));

$mc = new MongoClient($host, array("journal" => false), array("context" => $ctx));

echo "journal disabled by default\n";
$doc = array("doc" => "ument");
$mc->test->bug572->insert($doc);
$mc->test->bug572->update(array("_id" => $doc["_id"]), array("updated" => "doc"));
$mc->test->bug572->remove(array("_id" => $doc["_id"]));

echo "Setting it to true, per-query\n";
$doc = array("doc" => "ument");
$mc->test->bug572->insert($doc, array("j" => true));
$mc->test->bug572->update(array("_id" => $doc["_id"]), array("updated" => "doc"), array("j" => true));
$mc->test->bug572->remove(array("_id" => $doc["_id"]), array("j" => true));

$mc = new MongoClient($host, array("journal" => false, "w" => 0), array("context" => $ctx));

echo "journal disabled by default, and gle\n";
$doc = array("doc" => "ument");
$mc->test->bug572->insert($doc);
$mc->test->bug572->update(array("_id" => $doc["_id"]), array("updated" => "doc"));
$mc->test->bug572->remove(array("_id" => $doc["_id"]));

echo "Setting it to true, per-query, with gle=0\n";
$doc = array("doc" => "ument");
$mc->test->bug572->insert($doc, array("j" => true));
$mc->test->bug572->update(array("_id" => $doc["_id"]), array("updated" => "doc"), array("j" => true));
$mc->test->bug572->remove(array("_id" => $doc["_id"]), array("j" => true));

?>