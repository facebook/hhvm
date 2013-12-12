<?php
parse_str("a=1&b=2", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

function myfunc($val) {
	return $val . '_callback';
}
echo filter_input(INPUT_GET, 'a', FILTER_CALLBACK, array("options"=>'myfunc'));
echo "\n";
echo filter_input(INPUT_GET, 'b', FILTER_VALIDATE_INT);
echo "\n";
$data = "data";

echo filter_var($data, FILTER_CALLBACK, array("options"=>'myfunc'));
echo "\n";

$res = filter_input_array(INPUT_GET, array(
				'a' => array(
					'filter' => FILTER_CALLBACK,
					'options' => 'myfunc'
					),
				'b' => FILTER_VALIDATE_INT 
		)
	);

var_dump($res);
?>