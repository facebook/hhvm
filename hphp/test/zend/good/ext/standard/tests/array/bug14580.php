<?php
	$arr = array (b"foo\0bar" => b"foo\0bar");
	$key = key($arr);
	echo strlen($key), ': ';
	echo urlencode($key), "\n";
?>