<?hh <<__EntryPoint>> function main(): void {
$temp_filename = dirname(__FILE__)."/xmlwriter_set_indent_string_error.tmp";
    $fp = fopen($temp_filename, "w");
    fwrite ($fp, "Hi");
    fclose($fp);
$resource = xmlwriter_open_uri($temp_filename);
try { var_dump(xmlwriter_set_indent_string($resource)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
error_reporting(0);
$temp_filename = dirname(__FILE__)."/xmlwriter_set_indent_string_error.tmp";
unlink($temp_filename);
}
