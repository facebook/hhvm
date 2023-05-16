<?hh <<__EntryPoint>> function main(): void {
$temp_filename = sys_get_temp_dir().'/'.'xmlwriter_set_indent_string_error.tmp';
    $fp = fopen($temp_filename, "w");
    fwrite ($fp, "Hi");
    fclose($fp);
$resource = xmlwriter_open_uri($temp_filename);
try { var_dump(xmlwriter_set_indent_string($resource)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

unlink($temp_filename);
}
