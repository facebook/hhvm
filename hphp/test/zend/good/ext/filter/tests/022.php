<?php

var_dump(filter_var("a@b.c", FILTER_SANITIZE_EMAIL));
var_dump(filter_var("a[!@#$%^&*()@a@#$%^&*(.com@#$%^&*(", FILTER_SANITIZE_EMAIL));
var_dump(filter_var("white space here \ \ \" som more", FILTER_SANITIZE_EMAIL));
var_dump(filter_var("", FILTER_SANITIZE_EMAIL));
var_dump(filter_var("123456789000000", FILTER_SANITIZE_EMAIL));
	
echo "Done\n";
?>
