<?php
function iter($ar)
{
	foreach ($ar as $c) {
		echo htmlentities($c, 0, "UTF-8"), ": ", strlen($c), "\n";
	}
}
$teststr = "\xe2\x82\xac hi there";
iter(preg_split('//u', $teststr, -1, PREG_SPLIT_NO_EMPTY));
preg_match_all('/./u', $teststr, $matches);
iter($matches[0]);
?>