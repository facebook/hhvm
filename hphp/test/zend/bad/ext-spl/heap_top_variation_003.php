<?php
$h = new SplMinHeap();
try {
	$h->top();
} catch (Exception $e) {
	echo $e->getMessage();
}
?>