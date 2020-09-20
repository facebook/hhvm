<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('GMT0');
$tmp = "UYzymndjHGhgistwLBIW";
for($a = 0;$a < strlen($tmp); $a++){
    echo $tmp[$a], ': ', idate($tmp[$a], 1043324459)."\n";
}
}
