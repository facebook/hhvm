<?php
$dp = new DatePeriod('R5/2008-03-01T13:00:00Z/P1Y2M10DT2H30M');

foreach ($dp as $date) {
    echo $date->format("Y-m-d H:i:s\n");
}

echo "\n";

// this should repeat the same range
foreach ($dp as $date) {
    echo $date->format("Y-m-d H:i:s\n");
}
?>