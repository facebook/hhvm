<?php
parse_str("a=1%00", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);

echo ini_get('filter.default') . "\n";
echo ini_get('filter.default_flags') . "\n";
?>