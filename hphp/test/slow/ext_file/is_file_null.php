<?hh

<<__EntryPoint>>
function main_is_file_null() {
var_dump(is_dir(""));
var_dump(is_executable(""));
var_dump(is_file(""));
var_dump(is_link(""));
var_dump(is_readable(""));
var_dump(is_uploaded_file(""));
var_dump(is_writable(""));
var_dump(is_writeable(""));
}
