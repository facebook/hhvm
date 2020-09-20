<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set("UTC");

$tests = varray[
    'noon', 'midnight'
];

foreach ($tests as $test) {
    $t = strtotime("2005-12-22 ". $test);
    printf("%-10s => %s\n", $test, date(DATE_ISO8601, $t));
}
}
