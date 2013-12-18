<?php
parse_str("a=O'Henry&b=&c=<b>Bold</b>", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();

parse_str("d="quotes"&e=\slash", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);
_filter_snapshot_globals();
 echo $_GET['a'];
echo $_GET['b']; 
echo $_GET['c'];
echo $_POST['d'];
echo $_POST['e'];
echo "\n";
echo $_REQUEST['a'];
echo $_REQUEST['b'];
echo $_REQUEST['c'];
echo $_REQUEST['d'];
echo $_REQUEST['e'];
?>