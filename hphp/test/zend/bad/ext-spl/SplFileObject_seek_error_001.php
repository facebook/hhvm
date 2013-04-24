<?php
$obj = New SplFileObject(__FILE__);
$obj->seek(1,2);
$obj->seek();
try {
	$obj->seek(-1);
} catch (LogicException $e) {
	echo($e->getMessage());
}
?>