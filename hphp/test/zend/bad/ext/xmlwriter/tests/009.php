<?php 
/* $Id$ */
/*
Libxml 2.6.24 and up adds a new line after a processing instruction (PI)
*/
$xw = xmlwriter_open_memory();
xmlwriter_set_indent($xw, TRUE);
xmlwriter_start_document($xw, NULL, "UTF-8");
xmlwriter_start_element($xw, 'root');
xmlwriter_write_attribute($xw, 'id', 'elem1');
xmlwriter_start_element($xw, 'elem1');
xmlwriter_write_attribute($xw, 'attr1', 'first');
xmlwriter_write_comment($xw, 'start PI');
xmlwriter_start_element($xw, 'pi');
xmlwriter_write_pi($xw, 'php', 'echo "hello world"; ');
xmlwriter_end_element($xw);
xmlwriter_start_element($xw, 'cdata');
xmlwriter_start_cdata($xw);
xmlwriter_text($xw, '<>&"');
xmlwriter_end_cdata($xw);
xmlwriter_end_element($xw);
xmlwriter_end_element($xw);
xmlwriter_end_element($xw);
xmlwriter_end_document($xw);
// Force to write and empty the buffer
$output = xmlwriter_flush($xw, true);
print $output;
?>