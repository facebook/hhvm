<?php

function myfunc($val) {
	return $val . '_callback';
}
$data = "data";

echo filter_var($data, FILTER_CALLBACK, array("options"=>'myfunc'));
echo "\n";
