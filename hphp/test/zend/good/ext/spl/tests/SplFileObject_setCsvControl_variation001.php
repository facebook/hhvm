<?hh
<<__EntryPoint>>
function main(): void {
  chdir(sys_get_temp_dir());
  file_put_contents('SplFileObject_setCsvControl_variation001.csv',
<<<CDATA
"groene appelen",10
"gele bananen",20
"rode kersen",30
CDATA
);
$s = new SplFileObject('SplFileObject_setCsvControl_variation001.csv');
$s->setFlags(SplFileObject::READ_CSV);
$s->setCsvControl();
foreach ($s as $row) {
    list($fruit, $quantity) = $row;
    echo "$fruit : $quantity\n";
}
unlink('SplFileObject_setCsvControl_variation001.csv');
}
