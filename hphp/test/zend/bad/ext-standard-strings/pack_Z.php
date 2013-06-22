<?php
var_dump(
	pack("Z0", "f"),
    pack("Z5", "foo\0"),
    pack("Z4", "fooo"),
    pack("Z4", "foo"),
	pack("Z*", "foo"),
    unpack("Z*", "foo\0\rbar\0 \t\r\n"),
    unpack("Z9", "foo\0\rbar\0 \t\r\n"),
    unpack("Z2", "\0"),
    unpack("Z2", "\0\0"),
    unpack("Z2", "A\0"),
    unpack("Z2", "AB\0"),
    unpack("Z2", "ABC")
);