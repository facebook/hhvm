<?php
var_dump(bin2hex(mb_convert_encoding("\x80", 'UCS-2BE', 'CP936')));
var_dump(bin2hex(mb_convert_encoding("\x20\xac", 'CP936', 'UCS-2BE')));
?>