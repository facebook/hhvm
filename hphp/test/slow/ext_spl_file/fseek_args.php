<?hh


<<__EntryPoint>>
function main_fseek_args() {
$fname = tempnam(sys_get_temp_dir(), 'foobar');
file_put_contents($fname, 'herpderp');
$spl = new SplFileObject($fname, 'r');
$spl->fseek($spl->getSize() - 4);
var_dump($spl->fgets());
}
