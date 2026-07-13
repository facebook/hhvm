<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set("GMT");

$i = 5;
$max = getrandmax();
$max_2 = $max / 2;

while(true) {
    $t = $i; $i--;
    if (!$t) break;
    $new_tm = rand(1, $max);
    if ($new_tm > $max_2)
        $new_tm *= -1;

    if (strtotime("@$new_tm") != $new_tm) {
        echo "Error when parsing: @$new_tm\n";
    }
}

echo "done!";
}
