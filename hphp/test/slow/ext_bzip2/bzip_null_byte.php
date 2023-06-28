<?hh


<<__EntryPoint>>
function main_bzip_null_byte() :mixed{
$file = '/etc/passwd'.chr(0).'asdf';

var_dump(bzopen($file, 'r'));
}
