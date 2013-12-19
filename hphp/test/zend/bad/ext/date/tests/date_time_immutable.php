<?php
$tz = new DateTimeZone("Asia/Tokyo");
$current = "2012-12-27 16:24:08";

function dump($a, $b, $c)
{
	echo 'orig:    ', $a->format('Y-m-d H:i:s e'), "\n";
	echo 'copy:    ', $b->format('Y-m-d H:i:s e'), "\n";
	echo 'changed: ', $c->format('Y-m-d H:i:s e'), "\n";
}

echo "modify():\n";
$v = date_create_immutable($current);
$z = $v;
$x = $z->modify("+2 days");
dump($v, $z, $x);
$v = date_create($current);
$z = $v;
$x = $z->modify("+2 days");
dump($v, $z, $x);

echo "\nadd():\n";
$v = date_create_immutable($current);
$z = $v;
$x = $z->add(new DateInterval("P2DT2S"));
dump($v, $z, $x);
$v = date_create($current);
$z = $v;
$x = $z->add(new DateInterval("P2DT2S"));
dump($v, $z, $x);

echo "\nsub():\n";
$v = date_create_immutable($current);
$z = $v;
$x = $z->sub(new DateInterval("P2DT2S"));
dump($v, $z, $x);
$v = date_create($current);
$z = $v;
$x = $z->sub(new DateInterval("P2DT2S"));
dump($v, $z, $x);

echo "\nsetTimezone():\n";
$v = date_create_immutable($current);
$z = $v;
$x = $z->setTimezone($tz);
dump($v, $z, $x);
$v = date_create($current);
$z = $v;
$x = $z->setTimezone($tz);
dump($v, $z, $x);
$v = new DateTimeImmutable($current);
$z = $v;
$x = $z->setTimezone($tz);
dump($v, $z, $x);

echo "\nsetTime():\n";
$v = date_create_immutable($current);
$z = $v;
$x = $z->setTime(5, 7, 19);
dump($v, $z, $x);
$v = date_create($current);
$z = $v;
$x = $z->setTime(5, 7, 19);
dump($v, $z, $x);

echo "\nsetDate():\n";
$v = date_create_immutable($current);
$z = $v;
$x = $z->setDate(5, 7, 19);
dump($v, $z, $x);
$v = date_create($current);
$z = $v;
$x = $z->setDate(5, 7, 19);
dump($v, $z, $x);

echo "\nsetIsoDate():\n";
$v = date_create_immutable($current);
$z = $v;
$x = $z->setIsoDate(2012, 2, 6);
dump($v, $z, $x);
$v = date_create($current);
$z = $v;
$x = $z->setIsoDate(2012, 2, 6);
dump($v, $z, $x);

echo "\nsetTimestamp():\n";
$v = date_create_immutable($current);
$z = $v;
$x = $z->setTimestamp(2012234222);
dump($v, $z, $x);
$v = date_create($current);
$z = $v;
$x = $z->setTimestamp(2012234222);
dump($v, $z, $x);
?>