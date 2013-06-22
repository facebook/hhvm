<?php
parse_str("b=日本語0123456789日本語カタカナひらがな", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);

parse_str("a=日本語0123456789日本語カタカナひらがな", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);

// TODO: This is not a real test.... Need to change so that it does real testing
//$debug = true;
ini_set('include_path', dirname(__FILE__));
include_once('common.inc');

$ini = ini_get('mbstring.http_input');

// It must be url encoded....
// echo vars
echo $_POST['a']."\n";
echo $_GET['b']."\n";

// Get encoding
$enc = mb_http_input('P');

// check
if (empty($ini)) {
	// Must be pass
	if ($enc === 'pass') {
		echo "OK\n";
	}
	else {
		echo "NG\n";
	}
}
else {
	// Some encoding
	echo "This heppens when php.ini-dist is not used\n";
}

?>
