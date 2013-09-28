<?php
mb_internal_encoding("UCS-4LE");
var_dump(bin2hex(mb_strcut("\x61\x00\x00\x00\x62\x00\x00\x00\x63\x00\x00\x00", 0, 32)));
mb_internal_encoding("UCS-4BE");
var_dump(bin2hex(mb_strcut("\x00\x00\x00\x61\x00\x00\x00\x62\x00\x00\x00\x63", 0, 32)));
mb_internal_encoding("UCS-2LE");
var_dump(bin2hex(mb_strcut("\x61\x00\x62\x00\x63\x00", 0, 32)));
mb_internal_encoding("UCS-2BE");
var_dump(bin2hex(mb_strcut("\x00\x61\x00\x62\x00\x63", 0, 32)));
mb_internal_encoding("UTF-16");
var_dump(bin2hex(mb_strcut("\x00\x61\x00\x62\x00\x63", 0, 32)));
mb_internal_encoding("UTF-8");
var_dump(bin2hex(mb_strcut("abc", 0, 32)));
mb_internal_encoding("ISO-8859-1");
var_dump(bin2hex(mb_strcut("abc", 0, 32)));