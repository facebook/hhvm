<?php

var_dump(filter_var("}\"<p>test para</p>", FILTER_UNSAFE_RAW, FILTER_FLAG_ENCODE_AMP));
var_dump(filter_var("a[!@#<b>$%^&*()@a@#$%^&*(.<br>com@#$%^&*(", FILTER_UNSAFE_RAW, FILTER_FLAG_ENCODE_AMP));
var_dump(filter_var("white space here \ \ \" some more", FILTER_UNSAFE_RAW, FILTER_FLAG_ENCODE_AMP));
var_dump(filter_var("", FILTER_UNSAFE_RAW, FILTER_FLAG_ENCODE_AMP));
var_dump(filter_var("             123456789000000       <qwertyuiop> ", FILTER_UNSAFE_RAW, FILTER_FLAG_ENCODE_AMP));
	
echo "Done\n";
?>