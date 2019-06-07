<?hh <<__EntryPoint>> function main() {
ob_start();
while(@ob_end_clean());
var_dump(ob_get_clean());
}
