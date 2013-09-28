<?php
$s = '';
for($i=0; $i<256; $i++) {
	$s .= chr($i);
}
echo bin2hex($s)."\n";
echo bin2hex("abc")."\n";
?>