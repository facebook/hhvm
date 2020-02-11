<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('UTC');

$tests = varray[
    varray[ 'Y-m-d',   '2001-11-29 13:20:01' ],
    varray[ 'Y-m-d+',  '2001-11-29 13:20:01' ],
    varray[ 'Y-m-d +', '2001-11-29 13:20:01' ],
    varray[ 'Y-m-d+',  '2001-11-29' ],
    varray[ 'Y-m-d +', '2001-11-29' ],
    varray[ 'Y-m-d +', '2001-11-29 ' ],
];
foreach( $tests as $test )
{
    list($format, $str) = $test;
    var_dump($format, $str);
    $d = DateTime::createFromFormat($format, $str);
    var_dump($d);
    var_dump(DateTime::getLastErrors());

    echo "\n\n";
}
}
