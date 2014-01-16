<?php
	setlocale(LC_CTYPE, "fr_FR.ISO-8859-15", "fr_FR.ISO8859-15", 'fr_FR@euro');
	var_dump(htmlentities("\xbc\xbd\xbe", ENT_QUOTES, ''));
?>