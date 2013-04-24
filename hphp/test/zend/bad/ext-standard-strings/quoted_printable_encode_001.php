<?php

var_dump(quoted_printable_encode());
var_dump(quoted_printable_encode(""));
var_dump(quoted_printable_encode("test"));
var_dump(quoted_printable_encode("test", "more"));

$a = array("str");
var_dump(quoted_printable_encode($a));
var_dump(quoted_printable_encode(1));
var_dump(quoted_printable_encode(NULL));
var_dump(quoted_printable_encode(false));

echo "Done\n";
?>