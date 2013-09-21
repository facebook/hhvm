<?php 
/* $Id$ */

$xw = xmlwriter_open_memory();
xmlwriter_set_indent($xw, TRUE);
xmlwriter_set_indent_string($xw, '   ');
xmlwriter_start_document($xw, '1.0', "UTF-8");
xmlwriter_start_element($xw, 'root');
xmlwriter_start_element_ns($xw, 'ns1', 'child1', 'urn:ns1');
xmlwriter_start_attribute_ns($xw, 'ns1', 'att1', 'urn:ns1');
xmlwriter_text($xw, 'a&b');
xmlwriter_end_attribute($xw);
xmlwriter_write_attribute($xw, 'att2', "double\" single'");
xmlwriter_start_attribute_ns($xw, 'ns1', 'att2', 'urn:ns1');
xmlwriter_text($xw, "<>\"'&");
xmlwriter_end_attribute($xw);
xmlwriter_write_element($xw, 'chars', "special characters: <>\"'&");
xmlwriter_end_element($xw);
xmlwriter_end_document($xw);
// Force to write and empty the buffer
$output = xmlwriter_flush($xw, true);
print $output;
?>