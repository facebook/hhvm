<?php
var_dump(bin2hex(mb_convert_encoding("\xff\xfe\x01\x02\x03\x04", "UCS-2BE", "UTF-16")));
var_dump(bin2hex(mb_convert_encoding("\xfe\xff\x01\x02\x03\x04", "UCS-2BE", "UTF-16")));
var_dump(bin2hex(mb_convert_encoding("\xff\xfe\xff\xfe\x01\x02\x03\x04", "UCS-2BE", "UTF-16")));
var_dump(bin2hex(mb_convert_encoding("\xff\xfe\xfe\xff\x01\x02\x03\x04", "UCS-2BE", "UTF-16")));
var_dump(bin2hex(mb_convert_encoding("\xfe\xff\xff\xfe\x01\x02\x03\x04", "UCS-2BE", "UTF-16")));
var_dump(bin2hex(mb_convert_encoding("\xfe\xff\xfe\xff\x01\x02\x03\x04", "UCS-2BE", "UTF-16")));
?>