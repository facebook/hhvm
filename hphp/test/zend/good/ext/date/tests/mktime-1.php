<?hh
<<__EntryPoint>> function main(): void {
date_default_timezone_set("Europe/Amsterdam");

echo date("Y-m-d", mktime( 12, 0, 0, 3,  0, 2000)) ."\n";
echo date("Y-m-d", mktime( 12, 0, 0, 3, -1, 2000)) ."\n";
echo date("Y-m-d", mktime( 12, 0, 0, 2, 29, 2000)) ."\n";
echo date("Y-m-d", mktime( 12, 0, 0, 3,  0, 2001)) ."\n";
echo date("Y-m-d", mktime( 12, 0, 0, 2, 29, 2001)) ."\n";
echo date("Y-m-d", mktime( 12, 0, 0, 0,  0, 2000)) ."\n";

}
