<?php
	var_dump(stristr());
	var_dump(stristr(array(), ""));
	var_dump(stristr("", array()));
	var_dump(stristr(array(), array()));
	var_dump(stristr("tEsT sTrInG", "tEsT"));
	var_dump(stristr("tEsT sTrInG", "stRiNg"));
	var_dump(stristr("tEsT sTrInG", "stRiN"));
	var_dump(stristr("tEsT sTrInG", "t S"));
	var_dump(stristr("tEsT sTrInG", "g"));
	var_dump(md5(stristr("te".chr(0)."st", chr(0))));
	var_dump(@stristr("", ""));
	var_dump(@stristr("a", ""));
	var_dump(@stristr("", "a"));
	var_dump(md5(@stristr("\\\\a\\", "\\a")));
	var_dump(stristr("tEsT sTrInG", " "));
?>