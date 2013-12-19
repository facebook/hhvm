<?php
$streams = array(
	"data://text/plain,0\r\n",
	"data://text/plain,2\r\nte\r\n2\r\nst\r\n0\r\n",
	"data://text/plain,2\nte\n2\nst\n0\n",
	"data://text/plain,2;a=1\nte\n2;a=2;b=3\r\nst\n0\n",
	"data://text/plain,2\nte\n2\nst\n0\na=b\r\nc=d\n\r\n",
	"data://text/plain,1f\n0123456789abcdef0123456789abcde\n1\nf\n0\n",
	"data://text/plain,1E\n0123456789abcdef0123456789abcd\n2\nef\n0\n",
);
foreach ($streams as $name) {
	$fp = fopen($name, "r");
	stream_filter_append($fp, "dechunk", STREAM_FILTER_READ);
	var_dump(stream_get_contents($fp));
	fclose($fp);
}
?>