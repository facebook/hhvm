<?php

$ar = array(0, "123", 123, 22 => "abc", "a2b", 22, "a2d" => 7, 42);

foreach(new RegexIterator(new ArrayIterator($ar), "/2/") as $k => $v)
{
	echo "$k=>$v\n";
}

?>
===KEY===
<?php

foreach(new RegexIterator(new ArrayIterator($ar), "/2/", 0, RegexIterator::USE_KEY) as $k => $v)
{
	echo "$k=>$v\n";
}

?>
===DONE===
<?php exit(0); ?>
