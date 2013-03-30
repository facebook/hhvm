<?php

namespace foo;

$x = function (\stdclass $x = NULL) { 
	var_dump($x);	
};

$x(NULL);
$x(new \stdclass);

?>