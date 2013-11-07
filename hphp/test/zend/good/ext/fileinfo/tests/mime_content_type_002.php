<?php
	
var_dump(mime_content_type(__FILE__));
var_dump(mime_content_type(fopen(__FILE__, 'r')));
var_dump(mime_content_type('.'));
var_dump(mime_content_type('./..'));

?>