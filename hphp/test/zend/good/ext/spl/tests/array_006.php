<?php

echo "==Normal==\n";

$arr = array(0=>0, 1=>1, 2=>2);
$obj = new ArrayIterator($arr);

foreach($obj as $ak=>$av) {
	foreach($obj as $bk=>$bv) {
		if ($ak==0 && $bk==0) {
			$arr[0] = "modify";
		}
		echo "$ak=>$av - $bk=>$bv\n";
	}
}

?>
===DONE===
<?php exit(0); ?>
