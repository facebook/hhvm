<?php
	var_dump(strrpos("test test string", "test"));
	var_dump(strrpos("test string sTring", "string"));
	var_dump(strrpos("test strip string strand", "str"));
	var_dump(strrpos("I am what I am and that's all what I am", "am", -3));
	var_dump(strrpos("test string", "g"));
	var_dump(strrpos("te".chr(0)."st", chr(0)));
	var_dump(strrpos("tEst", "test"));
	var_dump(strrpos("teSt", "test"));
	var_dump(@strrpos("foo", "f", 1));
	var_dump(@strrpos("", ""));
	var_dump(@strrpos("a", ""));
	var_dump(@strrpos("", "a"));
	var_dump(@strrpos("\\\\a", "\\a"));
?>