<?php
ini_set('mbstring.internal_encoding', 'ISO-8859-15');
ini_set('mbstring.encoding_translation', true);
var_dump(mb_internal_encoding());
mb_internal_encoding('UTF-8');
var_dump(mb_internal_encoding());
parse_str('a=b');
var_dump(mb_internal_encoding());
mb_internal_encoding('UTF-8');
var_dump(mb_internal_encoding());
parse_str('a=b');
var_dump(mb_internal_encoding());
?>