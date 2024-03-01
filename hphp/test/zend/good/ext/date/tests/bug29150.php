<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('GMT');
echo gmdate("Y-m-d H:i:s", strtotime("20 VI. 2005"));
}
