<?php
$dirname = dirname(__FILE__) . '/';
$file = $dirname . 'test_with_comment.zip';
include $dirname . 'utils.inc';
$zip = new ZipArchive;
if (!$zip->open($file)) {
	exit('failed');
}
$fp = $zip->getStream('foo');

if(!$fp) exit("\n");

var_dump(stream_get_meta_data($fp));

fclose($fp);
$zip->close();


$fp = fopen('zip://' . dirname(__FILE__) . '/test_with_comment.zip#foo', 'rb');
if (!$fp) {
  exit("cannot open\n");
}

var_dump(stream_get_meta_data($fp));
fclose($fp);

?>