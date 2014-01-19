<?php

$array = array("foo"=>"bar","baz"=>1,"test"=>"a ' \" ", "abc");
var_dump(http_build_query($array));
var_dump(http_build_query($array, 'foo'));
var_dump(http_build_query($array, 'foo', ';'));

?>