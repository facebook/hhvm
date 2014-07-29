<?php
	mb_internal_encoding('Shift_JIS');
	print mb_internal_encoding()."\n";
	var_dump(bin2hex(htmlentities("\x81\x41\x81\x42\x81\x43", ENT_QUOTES, '')));
?>
===DONE===