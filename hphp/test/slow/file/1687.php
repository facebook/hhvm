<?hh


<<__EntryPoint>>
function main_1687() {
$tmp_dir=sys_get_temp_dir().'/tcr_tmp_dir'.getmypid();
mkdir($tmp_dir);
print_r(glob($tmp_dir.'/*'));
rmdir($tmp_dir);
}
