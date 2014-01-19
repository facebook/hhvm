<?php

$hdf = "bool = false\n".
"string = text\n".
"num = 12345\n".
"arr {\n".
"  bool = false\n".
"  string = anothertext\n".
"  num = 6789\n".
"}\n";

var_dump(parse_hdf_string($hdf));
