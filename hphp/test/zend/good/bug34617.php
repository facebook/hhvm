<?php
class Thing {}
function boom()
{
    $reader = xml_parser_create();
    xml_set_object($reader, new Thing());
    die("ok\n");
    xml_parser_free($reader);
}
boom();
?>