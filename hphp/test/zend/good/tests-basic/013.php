<?php
parse_str("a[]=1", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);

var_dump($_POST['a']); 
?>