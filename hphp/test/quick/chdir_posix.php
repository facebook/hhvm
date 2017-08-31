<?hh

$origdir = getcwd();

// Ugh. This (racy crap) is the 'recommended' way to fake tmpdir()
$dirname = tempnam(sys_get_temp_dir(), $prefix='hhvm_chdir');
unlink($dirname);
mkdir($dirname);

var_dump(chdir($dirname));
chdir($origdir);
chmod($dirname, 0600);
var_dump(chdir($dirname));
rmdir($dirname);
