<?php
parse_str("a[]=1&a[0]=5", $_POST);

var_dump($_POST['a']); 
?>