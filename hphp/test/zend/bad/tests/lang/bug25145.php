<?php
parse_str("123[]=SEGV", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();


var_dump($_REQUEST);
echo "Done\n";

?>