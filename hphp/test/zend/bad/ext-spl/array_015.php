<?php

$ar = new ArrayObject();

$ar[0] = 1;
$ar[1] = 2;
$ar[2] = 3;
$ar[3] = 4;
$ar[4] = 5;

var_dump($ar);

$it = $ar->getIterator();

$ar->offsetUnset($it->key());
$it->next();

var_dump($it->current());
var_dump($ar);

foreach($it as $k => $v)
{
	$ar->offsetUnset($k+1);
	echo "$k=>$v\n";
}

var_dump($ar);

foreach($it as $k => $v)
{
	$ar->offsetUnset($k);
	echo "$k=>$v\n";
}

var_dump($ar);

?>
===DONE===
<?php exit(0); ?>