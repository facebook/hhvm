<?php
$dp = new DatePeriod(new DateTime('2010-01-01 UTC'), new DateInterval('P1D'), 2);

echo "Original:\r\n";
foreach($dp as $dt) {
        echo $dt->format('Y-m-d H:i:s')."\r\n";
}
echo "\r\n";
var_dump($dp);

$ser = serialize($dp); // $ser is: O:10:"DatePeriod":0:{}

// Create dangerous instance
$dpu = unserialize($ser); // $dpu has invalid values???
var_dump($dpu);

echo "Unserialized:\r\n";
foreach($dpu as $dt) {
        echo $dt->format('Y-m-d H:i:s')."\r\n";
}
?>
==DONE==