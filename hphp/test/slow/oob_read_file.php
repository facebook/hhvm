<?hh

<<__EntryPoint>>
function main() :mixed{
    $a = bzopen("/dev/null", "w");
    $tmp = stream_get_line($a, 1, "1");
    var_dump($tmp);

    $a = fopen("php://output", "w");
    $tmp = stream_get_line($a, 1, "1");
    var_dump($tmp);
}
