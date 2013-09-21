<?php
parse_str("a=1", $_GET);
$_REQUEST = array_merge($_REQUEST, $_GET);
 echo $_GET['a']; ?>