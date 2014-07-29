<?php
xml_parse_into_struct(xml_parser_create_ns(), str_repeat("<blah>", 1000), $a);

echo "Done\n";
?>