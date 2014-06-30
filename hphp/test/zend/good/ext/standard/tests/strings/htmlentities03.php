<?php
	setlocale( LC_CTYPE, "de_DE.ISO-8859-1", "de_DE.ISO8859-1");
	var_dump(htmlentities("\xe4\xf6\xfc", ENT_QUOTES, ''));
?>