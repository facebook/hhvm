<?php
/* Prototype  : string json_encode  ( mixed $value  )
 * Description: Returns the JSON representation of a value
 * Source code: ext/json/php_json.c
 * Alias to functions: 
 */
echo "*** Testing json_encode() : basic functionality with UTF-8 input***\n";

$utf8_string = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII=');
var_dump(json_encode($utf8_string)); 

?>
===Done===