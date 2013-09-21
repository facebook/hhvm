<?php
parse_str("123[]=SEGV", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);


var_dump($_REQUEST);
echo "Done\n";

?>