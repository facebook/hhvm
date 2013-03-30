<?php

namespace foo;

$x = function (\stdclass $x = NULL) { 
	var_dump($x);	
};

class stdclass extends \stdclass { }

$x(NULL);
$x(new stdclass);
$x(new \stdclass);

?>