<?php
$str = <<< HERE
1:<!-- abc -  -->
2:<!doctype -- >
3:
4:<abc - def>
5:abc - def
6:</abc>

HERE;

echo strip_tags($str);
echo strip_tags($str, '<abc>');
?>