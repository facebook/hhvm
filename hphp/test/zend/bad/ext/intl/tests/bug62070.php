<?php
$s1 = 'Hello';

$coll = collator_create('en_US');
$res = collator_get_sort_key($coll, $s1);

echo urlencode($res);