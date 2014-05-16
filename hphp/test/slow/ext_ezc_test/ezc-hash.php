<?php

$ht = ezc_hash_create();
ezc_hash_set($ht, 'k', "a\n");
print "get(k): " . ezc_hash_get($ht, 'k');
ezc_hash_set($ht, 'k', "b\n");
print "get(k): " . ezc_hash_get($ht, 'k');
ezc_hash_append($ht, "c\n");
print "get(0): " . ezc_hash_get($ht, '0');
ezc_hash_append($ht, "d\n");
print "get(0): " . ezc_hash_get($ht, '0');
print "get(1): " . ezc_hash_get($ht, '1');
// Call destructors
$ht = null;
print "Done\n";
