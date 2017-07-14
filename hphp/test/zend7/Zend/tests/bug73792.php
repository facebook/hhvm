<?php
$a = 'aaa';

foreach ($a['bbb'] as &$value) {
	echo 'loop';
}

unset($value);
echo 'done';
?>
