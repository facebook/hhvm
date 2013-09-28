<?php

var_dump(preg_match('/x(.)/', 'fjszxax', $match, PREG_OFFSET_CAPTURE));
var_dump($match);

var_dump(preg_match('/(.)x/', 'fjszxax', $match, PREG_OFFSET_CAPTURE, 4));
var_dump($match);

var_dump(preg_match('/(?P<capt1>.)(x)(?P<letsmix>\S+)/', 'fjszxax', $match, PREG_OFFSET_CAPTURE));
var_dump($match);

?>