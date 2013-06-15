<?php

$ini =
";;; Created on Tuesday, October 27, 2009 at 12:01 PM GMT\n".
"[GJK_Browscap_Version]\n".
"Version=4520\n".
"Released=Tue, 27 Oct 2009 12:01:07 -0000\n".
"\n".
"\n;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DefaultProperties\n".
"\n".
"[DefaultProperties]\n".
"Browser=\"DefaultProperties\"\n".
"Version=0\n".
"Platform=unknown\n".
"Beta=false\n";

var_dump(parse_ini_string($ini));
echo "===\n";
var_dump(parse_ini_string($ini, true));
