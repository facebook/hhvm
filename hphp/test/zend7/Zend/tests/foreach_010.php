<?php
$o = (object)['a'=>0, 'b'=>1, 'c'=>2, 'd'=>3, 'e'=>4, 'f'=>5, 'g'=>6, 'h'=>7];
unset($o->a, $o->b, $o->c, $o->d);
foreach ($o as $v1) {
	foreach ($o as $v2) {
		echo "$v1-$v2\n";
		if ($v1 == 5 && $v2 == 6) {
			$o->i = 8;
		}	
	}
}
?>
