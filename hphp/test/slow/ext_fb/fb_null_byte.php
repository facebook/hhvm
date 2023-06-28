<?hh


<<__EntryPoint>>
function main_fb_null_byte() :mixed{
$file = '/etc/passwd'.chr(0).'asdf';

var_dump(fb_lazy_lstat($file));
var_dump(fb_lazy_realpath($file));
}
