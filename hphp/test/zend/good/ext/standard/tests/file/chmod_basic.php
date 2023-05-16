<?hh

const MODE_MASK = 07777;
<<__EntryPoint>> function main(): void {
$filename = sys_get_temp_dir().'/'.'chmod_basic.tmp';

$fd = fopen($filename, "w+");
fclose($fd);

for ($perms_to_set = 07777; $perms_to_set >= 0; $perms_to_set--) {
    chmod($filename, $perms_to_set);
    $set_perms = (fileperms($filename) & MODE_MASK);
    clearstatcache();
    if ($set_perms != $perms_to_set) {
        printf("Error: %o does not match %o\n", $set_perms, $perms_to_set);
    }
}
var_dump(chmod($filename, 0777));

unlink($filename);
echo "done";
}
