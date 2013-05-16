<?php

$array = parse_ini_string('
		int = 123
		constant = INSTALL_ROOT
		quotedString = "string"
		a = INSTALL_ROOT "waa"
		b = "INSTALL_ROOT"
		c = "waa" INSTALL_ROOT
		d = INSTALL_ROOT "INSTALL_ROOT"', false, INI_SCANNER_RAW);

var_dump($array);