<?php
parse_str("a[][]=1&a[][]=3&b[a][b][c]=1&b[a][b][d]=1", $_POST);

var_dump($_POST['a']); 
var_dump($_POST['b']); 
?>