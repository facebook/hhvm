<?hh <<__EntryPoint>> function main(): void {
$default_max = getrandmax();

echo "\nrand() tests with default min and max value (i.e 0 thru ", $default_max, ")\n";
for ($i = 0; $i < 100; $i++) {
    $res = rand();

// By default RAND_MAX is 32768 although no constant is defined for it for user space apps
    if (!is_int($res) || $res < 0 || $res > $default_max) {
        break;
    }
}

if ($i != 100) {
    echo "FAILED: res = ", $res, " min = 0 max = ", $default_max, "\n";
} else {
    echo "PASSED: range min = 0 max = ", $default_max, "\n";
}

echo "\nrand() tests with defined min and max value\n";

$min = vec[10,
             100,
             10.5,
             10.5e3,
             0x10,
             0400];

$max = vec[100,
             1000,
             19.5,
             10.5e5,
             0x10000,
             0700];

for ($x = 0; $x < count($min); $x++) {
    for ($i = 0; $i < 100; $i++) {
        $res = rand((int)$min[$x], (int)$max[$x]);

        if (!is_int($res) || $res < intval($min[$x]) || $res > intval($max[$x])) {
            echo "FAILED: res = ",  $res, " min = ", intval($min[$x]), " max = ", intval($max[$x]), "\n";
            break;
        }
    }

    if ($i == 100) {
        echo "PASSED: range min = ", intval($min[$x]), " max = ", intval($max[$x]), "\n";
    }
}

echo "\nNon-numeric cases\n";
$min = vec[true,
             false,
             null,
             "10",
             "0x10",
             "10.5"];

// Eexepcted numerical equivalent of above non-numerics
$minval = vec[1,
                0,
                0,
                10,
                0,
                10];
for ($x = 0; $x < count($min); $x++) {
    for ($i = 0; $i < 100; $i++) {
        $res = rand((int)$min[$x], 100);

        if (!is_int($res) || $res < intval($minval[$x]) || $res > 100) {
            echo "FAILED: res = ",  $res, " min = ", intval($min[$x]), " max = ", intval($max[$x]), "\n";
            break;
        }
    }

    if ($i == 100) {
        echo "PASSED range min = ", intval($min[$x]), " max = 100\n";
    }
}
}
