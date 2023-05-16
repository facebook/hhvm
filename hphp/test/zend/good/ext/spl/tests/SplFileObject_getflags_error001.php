<?hh
<<__EntryPoint>> function main(): void {
chdir(sys_get_temp_dir());
file_put_contents('SplFileObject_getflags_error001.csv', 'eerste;tweede;derde');


$fo = new SplFileObject('SplFileObject_getflags_error001.csv');
$fo->setFlags(SplFileObject::READ_CSV);

$fo->setFlags(SplFileObject::DROP_NEW_LINE);

var_dump($fo->getFlags());

unlink('SplFileObject_getflags_error001.csv');
}
