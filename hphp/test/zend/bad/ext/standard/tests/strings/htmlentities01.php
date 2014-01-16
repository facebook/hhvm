<?php
	var_dump(htmlentities(b"\x82\x86\x99\x9f", ENT_QUOTES, 'cp1252'));
	var_dump(htmlentities(b"\x80\xa2\xa3\xa4\xa5", ENT_QUOTES, 'cp1252'));
?>