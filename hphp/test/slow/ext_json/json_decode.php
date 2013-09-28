<?php


$arr = array("fbid" => 101501853510151001);
var_dump(json_decode(json_encode($arr), true));

var_dump(json_decode("{\"0\":{\"00\":0}}", true));

var_dump(json_decode("{\"a\":1,\"b\":2.3,\"3\":\"test\"}", true));
var_dump(json_decode("[\"a\",1,true,false,null]", true));

$obj = json_decode("[\"a\",1,true,false,null]");
var_dump($obj);

var_dump(json_decode("{z:1}",     true));
var_dump(json_decode("{z:1}",     true, JSON_FB_LOOSE));
var_dump(json_decode("{z:\"z\"}", true));
var_dump(json_decode("{z:\"z\"}", true, JSON_FB_LOOSE));
var_dump(json_decode("{'x':1}",   true));
var_dump(json_decode("{'x':1}",   true, JSON_FB_LOOSE));
var_dump(json_decode("{y:1,}",    true));
var_dump(json_decode("{y:1,}",    true, JSON_FB_LOOSE));
var_dump(json_decode("{,}",       true));
var_dump(json_decode("{,}",       true, JSON_FB_LOOSE));
var_dump(json_decode("[1,2,3,]",  true));
var_dump(json_decode("[1,2,3,]",  true, JSON_FB_LOOSE));
var_dump(json_decode("[,]",       true));
var_dump(json_decode("[,]",       true, JSON_FB_LOOSE));
var_dump(json_decode("[]",        true));
var_dump(json_decode("[]",        true, JSON_FB_LOOSE));
var_dump(json_decode("{}",        true));
var_dump(json_decode("{}",        true, JSON_FB_LOOSE));

var_dump(json_decode("[{\"a\":\"apple\"},{\"b\":\"banana\"}]", true));

$a = "[{\"a\":[{\"n\":\"1st\"}]},{\"b\":[{\"n\":\"2nd\"}]}]";
var_dump(json_decode($a, true));
