<?php
	mb_internal_encoding('ISO-8859-15');
	print mb_internal_encoding()."\n";
	var_dump(htmlentities("\xbc\xbd\xbe", ENT_QUOTES, ''));
?>