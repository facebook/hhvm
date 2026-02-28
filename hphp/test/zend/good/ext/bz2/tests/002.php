<?hh
<<__EntryPoint>> function main(): void {
chdir(sys_get_temp_dir());

$fp = fopen("bz_open_002.txt", "w");
var_dump(bzopen($fp, "w"));

$fp = fopen("bz_open_002.txt", "r");
var_dump(bzopen($fp, "r"));

unlink("bz_open_002.txt");
$fp = fopen("bz_open_002.txt", "x");
var_dump(bzopen($fp, "w"));

unlink("bz_open_002.txt");
$fp = fopen("bz_open_002.txt", "x");
var_dump(bzopen($fp, "r"));

$fp = fopen("bz_open_002.txt", "rb");
var_dump(bzopen($fp, "r"));

$fp = fopen("bz_open_002.txt", "wb");
var_dump(bzopen($fp, "w"));

$fp = fopen("bz_open_002.txt", "br");
var_dump(bzopen($fp, "r"));

$fp = fopen("bz_open_002.txt", "br");
var_dump(bzopen($fp, "w"));

$fp = fopen("bz_open_002.txt", "r");
var_dump(bzopen($fp, "w"));

$fp = fopen("bz_open_002.txt", "w");
var_dump(bzopen($fp, "r"));

$fp = fopen("bz_open_002.txt", "rw");
var_dump(bzopen($fp, "w"));

$fp = fopen("bz_open_002.txt", "rw");
var_dump(bzopen($fp, "r"));

$fp = fopen("bz_open_002.txt", "wr");
var_dump(bzopen($fp, "w"));

$fp = fopen("bz_open_002.txt", "wr");
var_dump(bzopen($fp, "r"));

$fp = fopen("bz_open_002.txt", "r+");
var_dump(bzopen($fp, "r"));

$fp = fopen("bz_open_002.txt", "r+");
var_dump(bzopen($fp, "w"));

$fp = fopen("bz_open_002.txt", "w+");
var_dump(bzopen($fp, "r"));

$fp = fopen("bz_open_002.txt", "w+");
var_dump(bzopen($fp, "w"));

$fp = fopen("bz_open_002.txt", "a");
var_dump(bzopen($fp, "r"));

$fp = fopen("bz_open_002.txt", "a");
var_dump(bzopen($fp, "w"));

unlink("bz_open_002.txt");

echo "Done\n";
}
