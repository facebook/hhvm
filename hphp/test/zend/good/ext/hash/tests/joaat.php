<?hh <<__EntryPoint>> function main(): void {
$tests = vec[
    vec["hello world", "3e4a5a57"],
    vec["", 0],
    vec["", "000000"],
    vec["a", "ca2e9442"],
    vec["aa", "7081738e"],
];

$i = 0;
$pass = true;

foreach ($tests as $test) {
    ++$i;

    $result = hash("joaat", $test[0]);
    if (HH\Lib\Legacy_FIXME\neq($result, $test[1])) {
        echo "Iteration " . $i . " failed - expected '" . $test[1] . "', got '" . $result . "' for '" . $test[1] . "'\n";

        $pass = false;
    }
}

if($pass) {
    echo "PASS";
}
}
