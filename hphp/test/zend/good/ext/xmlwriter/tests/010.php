<?hh
<<__EntryPoint>> function main(): void {
$file = sys_get_temp_dir().'/'.'010.tmp';

$xw = xmlwriter_open_uri($file);

var_dump(xmlwriter_start_element($xw, "tag"));
var_dump(xmlwriter_start_attribute($xw, "attr"));
var_dump(xmlwriter_end_attribute($xw));
var_dump(xmlwriter_start_attribute($xw, "-1"));
var_dump(xmlwriter_end_attribute($xw));
var_dump(xmlwriter_start_attribute($xw, "\""));
var_dump(xmlwriter_end_attribute($xw));
var_dump(xmlwriter_end_element($xw));

unset($xw);

var_dump(file_get_contents($file));

unlink($file);

echo "Done\n";
}
