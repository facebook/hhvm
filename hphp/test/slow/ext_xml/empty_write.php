<?hh

<<__EntryPoint>>
function main_empty_write() {
$fname = getenv('HPHP_TEST_TMPDIR') . 'xmlout';
$writer = xmlwriter_open_uri($fname);
xmlwriter_flush($writer);
print file_exists($fname) ? "written\n" : "no file\n";
unlink($fname);
}
