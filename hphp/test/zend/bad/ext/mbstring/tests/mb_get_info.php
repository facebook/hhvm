<?php
mb_parse_str("abc=def", $dummy);
mb_convert_encoding("\xff\xff", "Shift_JIS", "UCS-2BE");
$result = mb_get_info();
var_dump($result);
foreach (array_keys($result) as $key) {
    var_dump($result[$key], mb_get_info($key));
}
?>