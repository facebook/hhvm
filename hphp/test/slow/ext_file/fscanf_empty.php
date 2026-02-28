<?hh

<<__EntryPoint>>
function main_fscanf_empty() :mixed{
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

touch($tempfile);
$f = fopen($tempfile, "r");
$res = fscanf($f, "%s %s");
var_dump($res);

unlink($tempfile);
}
