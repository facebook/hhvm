<?php
	var_dump(nl2br("test"));
	var_dump(nl2br(""));
	var_dump(nl2br(NULL));
	var_dump(nl2br("\r\n"));
	var_dump(nl2br("\n"));
	var_dump(nl2br("\r"));
	var_dump(nl2br("\n\r"));
	
	var_dump(nl2br("\n\r\r\n\r\r\r\r"));
	var_dump(nl2br("\n\r\n\n\r\n\r\r\n\r\n"));
	var_dump(nl2br("\n\r\n\n\n\n\r\r\r\r\n\r"));
	
?>