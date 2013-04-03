<?php
parse_str("a[]=1&a[a]=1&a[b]=3", $_POST);

var_dump($_POST['a']); 
?>