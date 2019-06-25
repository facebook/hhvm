<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set("GMT");
var_dump(strtotime("monkey"));
print date("Y-m-d", (int)strtotime("monkey")) ."\n";
print date("Y-m-d", 0) ."\n";
}
