<?php
            
$pattern =
"/\s([\w_\.\/]+)(?:=([\'\"]?(?:[\w\d\s\?=\(\)\.,'_#\/\\:;&-]|(?:\\\\\"|\\\')?)+[\'\"]?))?/";
$context = "<simpletag an_attribute=\"simpleValueInside\">";

$match = array();

if ($result =preg_match_all($pattern, $context, $match))
{

var_dump($result);
var_dump($match);
}

?>