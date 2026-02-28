<?hh
// fopen with interesting windows paths.
<<__EntryPoint>> function main(): void {
$testdir = sys_get_temp_dir().'/'.'bug47177.tmpdir';
mkdir($testdir);
$t = time() - 3600;
touch($testdir, $t);
clearstatcache();
$t2 = filemtime($testdir);
if ($t2 != $t) echo "failed (got $t2, expecting $t)\n";
rmdir($testdir);
echo "Ok.";
}
