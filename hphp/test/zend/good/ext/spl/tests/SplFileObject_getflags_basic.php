<?hh
<<__EntryPoint>> function main(): void {
chdir(__SystemLib\hphp_test_tmproot());
file_put_contents('SplFileObject_getflags_basic.csv', 'eerste;tweede;derde');

$fo = new SplFileObject('SplFileObject_getflags_basic.csv');

$fo->setFlags(SplFileObject::DROP_NEW_LINE);
var_dump($fo->getFlags());

unlink('SplFileObject_getflags_basic.csv');
}
