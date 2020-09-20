<?hh <<__EntryPoint>> function main(): void {
$tests = varray[
    varray["hello world", "3e4a5a57"],
    varray["", 0],
    varray["", "000000"],
    varray["a", "ca2e9442"],
    varray["aa", "7081738e"],
];

$i = 0;
$pass = true;

foreach ($tests as $test) {
    ++$i;

    $result = hash("joaat", $test[0]);
    if ($result != $test[1]) {
        echo "Iteration " . $i . " failed - expected '" . $test[1] . "', got '" . $result . "' for '" . $test[1] . "'\n";

        $pass = false;
    }
}

if($pass) {
    echo "PASS";
}
}
