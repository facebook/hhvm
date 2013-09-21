<?php

$values = Array(
'http://example.com/index.html',	
'http://www.example.com/index.php',	
'http://www.example/img/test.png',	
'http://www.example/img/dir/',	
'http://www.example/img/dir',	
'http//www.example/wrong/url/',	
'http:/www.example',	
'file:///tmp/test.c',	
'ftp://ftp.example.com/tmp/',	
'/tmp/test.c',	
'/',	
'http://',	
'http:/',	
'http:',	
'http',	
'',	
-1,	
array(),	
'mailto:foo@bar.com',
'news:news.php.net',
'file://foo/bar',
"http://\r\n/bar",
"http://example.com:qq",
"http://example.com:-2",
"http://example.com:65536",
"http://example.com:65537",
);
foreach ($values as $value) {
	var_dump(filter_var($value, FILTER_VALIDATE_URL));
}


var_dump(filter_var("qwe", FILTER_VALIDATE_URL, FILTER_FLAG_SCHEME_REQUIRED));
var_dump(filter_var("http://qwe", FILTER_VALIDATE_URL, FILTER_FLAG_SCHEME_REQUIRED));
var_dump(filter_var("http://", FILTER_VALIDATE_URL, FILTER_FLAG_HOST_REQUIRED));
var_dump(filter_var("/tmp/test", FILTER_VALIDATE_URL, FILTER_FLAG_HOST_REQUIRED));
var_dump(filter_var("http://www.example.com", FILTER_VALIDATE_URL, FILTER_FLAG_HOST_REQUIRED));
var_dump(filter_var("http://www.example.com", FILTER_VALIDATE_URL, FILTER_FLAG_PATH_REQUIRED));
var_dump(filter_var("http://www.example.com/path/at/the/server/", FILTER_VALIDATE_URL, FILTER_FLAG_PATH_REQUIRED));
var_dump(filter_var("http://www.example.com/index.html", FILTER_VALIDATE_URL, FILTER_FLAG_QUERY_REQUIRED));
var_dump(filter_var("http://www.example.com/index.php?a=b&c=d", FILTER_VALIDATE_URL, FILTER_FLAG_QUERY_REQUIRED));

echo "Done\n";
?>