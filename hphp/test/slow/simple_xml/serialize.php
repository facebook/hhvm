<?php

$element = new \SimpleXMLElement("<foo><bar>baz</bar></foo>");
var_dump(json_encode($element));
