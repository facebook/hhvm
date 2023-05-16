<?hh
<<__EntryPoint>> function main(): void {
chdir(sys_get_temp_dir());
mkdir("bug45181_x");
var_dump(is_dir("bug45181_x"));
chdir("bug45181_x");
var_dump(is_dir("bug45181_x"));

rmdir(sys_get_temp_dir().'/'.'bug45181_x');
}
