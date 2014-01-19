<?php
$o1 = new stdClass;
$o2 = new stdClass;

$s = new splObjectStorage();

$s->attach($o1, array('prev' => 2, 'next' => $o2));
$s->attach($o2, array('prev' => $o1));

$ss = serialize($s);
unset($s,$o1,$o2);
echo $ss."\n";
var_dump(unserialize($ss));
?>
===DONE===