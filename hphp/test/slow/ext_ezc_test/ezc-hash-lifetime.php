<?php

// Make sure the hash is destroyed automatically when the request ends. Should
// see the values printed after Done is printed.

$ht = ezc_hash_create();
ezc_hash_set($ht, 'k', "b\n");
ezc_hash_append($ht, "c\n");
ezc_hash_append($ht, "d\n");
print "Done\n";
