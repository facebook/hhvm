<?php
parse_str("a=1&b=&c=3", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
 echo $_GET['a'];
echo $_GET['b']; 
echo $_GET['c'];
?>