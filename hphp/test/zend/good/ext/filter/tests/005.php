<?php
parse_str("id=f03_photos&pgurl=http%3A//fifaworldcup.yahoo.com/03/en/photozone/index.html", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
_filter_snapshot_globals();
 
echo $_GET['id'];
echo "\n";
echo $_GET['pgurl']; 
echo "\n";
echo $_REQUEST['id'];
echo "\n";
echo $_REQUEST['pgurl']; 
?>