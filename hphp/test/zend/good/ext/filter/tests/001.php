<?php
parse_str("a=1", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();
 echo $_GET['a']; ?>