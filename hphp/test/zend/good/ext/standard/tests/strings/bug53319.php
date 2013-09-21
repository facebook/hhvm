<?php

$str = '<br /><br  />USD<input type="text"/><br/>CDN<br><input type="text" />';
var_dump(strip_tags($str, '<input>'));
var_dump(strip_tags($str, '<br><input>') === $str);
var_dump(strip_tags($str));
var_dump(strip_tags('<a/b>', '<a>'));

?>