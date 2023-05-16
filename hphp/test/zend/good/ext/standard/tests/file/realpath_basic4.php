<?hh <<__EntryPoint>> function main(): void {
@mkdir(sys_get_temp_dir().'/'.'realpath_basic4/home/test', 0777, true);
@symlink(
  sys_get_temp_dir().'/'.'realpath_basic4/home',
  sys_get_temp_dir().'/'.'realpath_basic4/link1'
);
@symlink(
  sys_get_temp_dir().'/'.'realpath_basic4/link1',
  sys_get_temp_dir().'/'.'realpath_basic4/link2'
);
echo "1. " . realpath(sys_get_temp_dir().'/'.'realpath_basic4/link2') . "\n";
echo "2. " . realpath(sys_get_temp_dir().'/'.'realpath_basic4/link2/test') . "\n";

unlink(sys_get_temp_dir().'/'.'realpath_basic4/link2');
unlink(sys_get_temp_dir().'/'.'realpath_basic4/link1');
rmdir(sys_get_temp_dir().'/'.'realpath_basic4/home/test');
rmdir(sys_get_temp_dir().'/'.'realpath_basic4/home');
rmdir(sys_get_temp_dir().'/'.'realpath_basic4');
}
