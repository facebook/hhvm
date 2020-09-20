<?hh
<<__EntryPoint>> function main(): void {
var_dump(chmod('', 0777));
var_dump(touch(''));
var_dump(disk_free_space(''));
}
