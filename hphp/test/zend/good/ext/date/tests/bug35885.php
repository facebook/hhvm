<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set("UTC");

$time = time();
$ts = date(DATE_ISO8601, strtotime('NOW', $time));
$ts2 = date(DATE_ISO8601, $time);

$res = ($ts == $ts2);
var_dump($res);

if (!$res) {
    var_dump($ts);
    var_dump($ts2);
}
}
