<?hh <<__EntryPoint>> function main(): void {
$frombase = varray[2,8,10,16,36];
$tobase = varray[2,8,10,16,36];

$values = varray[10,
                27,
                39,
                039,
                0x5F,
                "10",
                "27",
                "39",
                "5F",
                "3XYZ"
                ];

for ($f= 0; $f < count($frombase); $f++) {
    echo "\n...from base is ", $frombase[$f], "\n";
    for ($t= 0; $t < count($tobase); $t++) {
        echo "......to base is ", $tobase[$t], "\n";
        for ($i =0; $i < count($values); $i++){
            $res = base_convert($values[$i],$frombase[$f],$tobase[$t]);
            echo ".........value= ", $values[$i], " res = ", $res, "\n";
        }
    }
}
}
