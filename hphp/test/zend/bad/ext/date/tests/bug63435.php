<?php
for ($i=1 ; $i<999 ; $i++) {
	$datetime = Datetime::createFromFormat("u", sprintf("%06ld", $i));
	$res = $datetime->format("u");
	if ($res != $i) {
		echo "$i != $res\n";
	}
}
echo "Done";