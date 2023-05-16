<?hh // $Id$
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

$filename = sys_get_temp_dir().'/'.'with_files.bz';
$str = "This is a test string.\n";
$bz = bzopen($filename, "w");
bzwrite($bz, $str);
bzclose($bz);

$bz = bzopen($filename, "r");
print bzread($bz, 10);
print bzread($bz);
bzclose($bz);
unlink($filename);
}
