<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set("UTC");

$days = vec["monday","mon","tuesday","tue","wednesday","wed","thursday","thu","friday","fri","saturday","sat","sunday","sun"];

foreach ($days as $day) {
    echo date("D", strtotime($day));
    echo date("D", strtotime(ucfirst($day)));
    echo "\n";
}
}
