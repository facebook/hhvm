<?php
	var_dump(stripos("test string", "TEST"));
	var_dump(stripos("test string", "strIng"));
	var_dump(stripos("test string", "stRin"));
	var_dump(stripos("test string", "t S"));
	var_dump(stripos("test string", "G"));
	var_dump(stripos("te".chr(0)."st", chr(0)));
	var_dump(stripos("tEst", "test"));
	var_dump(stripos("teSt", "test"));
	var_dump(stripos("", ""));
	var_dump(stripos("a", ""));
	var_dump(stripos("", "a"));
	var_dump(stripos("a", " "));
	var_dump(stripos("a", "a"));
	var_dump(stripos("", 1));
	var_dump(stripos("", false));
	var_dump(stripos("", true));
	var_dump(stripos("a", 1));
	var_dump(stripos("a", false));
	var_dump(stripos("a", true));
	var_dump(stripos("1", 1));
	var_dump(stripos("0", false));
	var_dump(stripos("1", true));
	var_dump(stripos("\\\\a", "\\a"));

	echo "Done\n";
?>