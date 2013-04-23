<?php
$str = 'abc 123 #",; $foo';
echo mb_ereg_replace_callback('(\S+)', function($m){return $m[1].'('.strlen($m[1]).')';}, $str);
?>