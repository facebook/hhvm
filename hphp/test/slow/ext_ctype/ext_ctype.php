<?php

var_dump(ctype_alnum("abc123"));
var_dump(!ctype_alnum("!@#$^"));

var_dump(ctype_alpha("abcdef"));
var_dump(!ctype_alpha("abc123"));

var_dump(ctype_cntrl("\t\n\r"));
var_dump(!ctype_cntrl("abc123"));

var_dump(ctype_digit("123456"));
var_dump(!ctype_digit("abc123"));

var_dump(ctype_graph("!@#$^"));
var_dump(!ctype_graph("\x07"));

var_dump(ctype_lower("abcdef"));
var_dump(!ctype_lower("ABCDEF"));

var_dump(ctype_print("!@#$^"));
var_dump(!ctype_print("\x07"));

var_dump(ctype_punct("!@#$^"));
var_dump(!ctype_punct("ABCDEF"));

var_dump(ctype_space(" "));
var_dump(!ctype_space("a "));

var_dump(ctype_upper("ABCDEF"));
var_dump(!ctype_upper("abcdef"));

var_dump(ctype_xdigit("ABCDEF"));
var_dump(!ctype_xdigit("GHIJKL"));
