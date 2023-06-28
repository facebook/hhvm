<?hh

<<__EntryPoint>>
function main_empty_write() :mixed{
$fname = sys_get_temp_dir().'/'.'xmlout';
$writer = xmlwriter_open_uri($fname);
xmlwriter_flush($writer);
print file_exists($fname) ? "written\n" : "no file\n";
unlink($fname);
}
