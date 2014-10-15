<?php 
$temp_filename = dirname(__FILE__)."/xmlwriter_set_indent_string.tmp";
	$fp = fopen($temp_filename, "w");
	fwrite ($fp, "Hi");
	fclose($fp);
$resource = xmlwriter_open_uri($temp_filename);
var_dump(xmlwriter_set_indent_string($resource, '  '));
?>
<?php error_reporting(0); ?>
<?php
$temp_filename = dirname(__FILE__)."/xmlwriter_set_indent_string.tmp";
unlink($temp_filename);
?>