<?php
$xw = xmlwriter_open_memory();
xmlwriter_set_indent($xw, TRUE);
xmlwriter_start_document($xw, NULL, "UTF-8");
xmlwriter_start_element($xw, 'root');
xmlwriter_write_attribute_ns($xw, 'prefix', '', 'http://www.php.net/uri');
xmlwriter_start_element($xw, 'elem1');
xmlwriter_write_attribute($xw, 'attr1', 'first');
xmlwriter_end_element($xw);
xmlwriter_full_end_element($xw);
xmlwriter_end_document($xw);

$output = xmlwriter_flush($xw, true);
print $output;

// write attribute_ns without start_element first
$xw = xmlwriter_open_memory();
var_dump(xmlwriter_write_attribute_ns($xw, 'prefix', 'id', 'http://www.php.net/uri', 'elem1'));
print xmlwriter_output_memory($xw);
?>