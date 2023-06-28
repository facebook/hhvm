<?hh

<<__EntryPoint>>
function main_spl_file_object_fgets_dro_p_ne_w_line() :mixed{
$f = new SplFileObject("data://,line 1\n");
$f->setFlags(SplFileObject::DROP_NEW_LINE);
var_dump($f->fgets());
}
