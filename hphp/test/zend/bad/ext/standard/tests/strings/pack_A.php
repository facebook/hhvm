<?php
var_dump(
	pack("A5", "foo "),
	pack("A4", "fooo"),
	pack("A4", "foo"),
	unpack("A*", "foo\0\rbar\0 \t\r\n"),
	unpack("A4", "foo\0\rbar\0 \t\r\n")
);
?>