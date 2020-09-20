<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set("GMT");
echo date('Y-W', strtotime('2005-1-1')), "\n";
echo date('o-W', strtotime('2005-1-1')), "\n";
}
