<?php
$data = str_repeat("a", 100);
for ($i = 0xF0; $i < 0xFF + 1; $i++) {
	$enc = chunk_split(base64_encode($data), 10, chr($i));
	var_dump(base64_decode($enc));
}
?>