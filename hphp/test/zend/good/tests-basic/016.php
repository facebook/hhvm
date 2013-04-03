<?php
parse_str("a[a]=1&a[b]=3", $_POST);

var_dump($_POST['a']); 
?>