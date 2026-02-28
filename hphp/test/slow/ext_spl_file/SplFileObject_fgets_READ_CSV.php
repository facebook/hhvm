<?hh

<<__EntryPoint>>
function main_spl_file_object_fgets_rea_d_csv() :mixed{
$f = new SplFileObject("data://,a,b,c,line1\n");
$f->setFlags(SplFileObject::READ_CSV);
var_dump($f->fgets());
}
