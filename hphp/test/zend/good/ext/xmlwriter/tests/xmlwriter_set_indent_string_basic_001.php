<?hh
<<__EntryPoint>> function main(): void {
$temp_filename = sys_get_temp_dir().'/'.'xmlwriter_set_indent_string.tmp';
    $fp = fopen($temp_filename, "w");
    fwrite ($fp, "Hi");
    fclose($fp);
$resource = xmlwriter_open_uri($temp_filename);
var_dump(xmlwriter_set_indent_string($resource, '  '));

unlink($temp_filename);
}
