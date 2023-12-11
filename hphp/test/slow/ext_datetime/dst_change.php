<?hh


<<__EntryPoint>>
function main_dst_change() :mixed{
$datetimes = vec[
    '2014-10-05T01:59:59',
    '2014-10-05T02:00:00',
    '2014-10-05T02:59:59',
    '2014-10-05T03:00:00'
];

foreach ($datetimes as $datetime) {
    $dt = new DateTime($datetime, new DateTimeZone('Australia/Sydney'));
    echo $dt->format(DateTime::ISO8601) . PHP_EOL;
}
}
