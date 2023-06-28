<?hh


<<__EntryPoint>>
function main_set_indent_string_error() :mixed{
$writer = xmlwriter_open_memory();
xmlwriter_set_indent_string($writer, '#');
// The second arg is supposedly required. But just keep executing anyway and use
// the previous value...
try { xmlwriter_set_indent_string($writer); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
xmlwriter_set_indent($writer, true);
xmlwriter_start_document($writer, '1.0');
xmlwriter_start_element($writer, 'foo');
xmlwriter_start_element($writer, 'bar');
xmlwriter_end_element($writer);
xmlwriter_end_element($writer);
xmlwriter_end_document($writer);

var_dump(xmlwriter_output_memory($writer));
}
