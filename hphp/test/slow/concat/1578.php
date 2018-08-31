<?php


<<__EntryPoint>>
function main_1578() {
$v = 1;
echo $v . b'a' . b"b" . `ls \055\144 \x2ftmp`;
echo b'a' . b"b" . `ls \055\144 \x2ftmp` . $v;
}
