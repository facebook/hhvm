<?php
	mb_internal_encoding('ISO-8859-1');
	print mb_internal_encoding()."\n";
	var_dump(htmlentities("\xe4\xf6\xfc", ENT_QUOTES, ''));
?>