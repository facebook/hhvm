<?hh

<<__EntryPoint>>
function main_file_uploads() :mixed{
var_dump(ini_get('file_uploads'));
var_dump(ini_get('upload_tmp_dir'));
}
