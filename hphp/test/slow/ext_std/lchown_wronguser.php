<?hh


<<__EntryPoint>>
function main_lchown_wronguser() :mixed{
$file = tempnam(sys_get_temp_dir(), 'lchown');
$link = tempnam(sys_get_temp_dir(), 'lchown');
touch($file);
@symlink($file, $link);

var_dump(lchown($link, 'ihopenomachinehasthisuserthatwouldbebad'));
var_dump(chown($file, 'ihopenomachinehasthisuserthatwouldbebad'));
var_dump(lchgrp($link, 'ihopenomachinehasthisgroupthatwouldbebad'));
var_dump(chgrp($file, 'ihopenomachinehasthisgroupthatwouldbebad'));
}
