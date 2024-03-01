<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('GMT');
echo strtotime("19:30 Dec 17 2005"), "\n";
echo strtotime("Dec 17 19:30 2005"), "\n";
}
