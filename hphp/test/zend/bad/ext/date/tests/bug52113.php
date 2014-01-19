<?php
$start = new DateTime('2003-01-02 08:00:00');
$end = new DateTime('2003-01-02 12:00:00');
$diff = $start->diff($end);
$p = new DatePeriod($start, $diff, 2);
$diff_s = serialize($diff);
var_dump($diff, $diff_s);
var_export($diff);

$diff_un = unserialize($diff_s);
$p = new DatePeriod($start, $diff_un, 2);
var_dump($diff_un, $p);

$unser = DateInterval::__set_state(array(
   'y' => 7,
   'm' => 6,
   'd' => 5,
   'h' => 4,
   'i' => 3,
   's' => 2,
   'invert' => 1,
   'days' => 2400,
));

$p = new DatePeriod($start, $diff_un, 2);
var_dump($unser, $p);

?>