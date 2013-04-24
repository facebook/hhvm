<?php

$xmlwriter = xmlwriter_open_memory();
var_dump(xmlwriter_write_dtd($xmlwriter, 'bla1', 'bla2', 'bla3', 'bla4'));
$output = xmlwriter_flush($xmlwriter, true);
print $output . PHP_EOL;

var_dump(xmlwriter_write_dtd($xmlwriter, '', '', ''));
$output = xmlwriter_flush($xmlwriter, true);
print $output;
?>