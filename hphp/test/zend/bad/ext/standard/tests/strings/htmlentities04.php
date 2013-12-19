<?php
	setlocale( LC_CTYPE, "ja_JP.EUC-JP", "ja_JP.eucJP" );
	var_dump(htmlentities("\xa1\xa2\xa1\xa3\xa1\xa4", ENT_QUOTES, ''));
?>