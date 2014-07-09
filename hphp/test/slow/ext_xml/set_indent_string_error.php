<?php

$writer = xmlwriter_open_memory();
xmlwriter_set_indent_string($writer, '#');
// The second arg is supposedly required. But just keep executing anyway and use
// the previous value...
xmlwriter_set_indent_string($writer);
xmlwriter_set_indent($writer, true);
xmlwriter_start_document($writer, '1.0');
xmlwriter_start_element($writer, 'foo');
xmlwriter_start_element($writer, 'bar');
xmlwriter_end_element($writer);
xmlwriter_end_element($writer);
xmlwriter_end_document($writer);

var_dump(xmlwriter_output_memory($writer));
