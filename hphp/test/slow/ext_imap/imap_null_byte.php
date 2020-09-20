<?hh


<<__EntryPoint>>
function main_imap_null_byte() {
$file = '/etc/passwd'.chr(0).'asdf';

var_dump(imap_open($file, 'user', 'pass'));
}
