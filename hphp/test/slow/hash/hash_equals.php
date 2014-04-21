<?php
// Adapted from ext/hash/tests/hash_equals.php to remove clowny error tests
var_dump(hash_equals("same", "same"));
var_dump(hash_equals("not1same", "not2same"));
var_dump(hash_equals("short", "longer"));
var_dump(hash_equals("longer", "short"));
var_dump(hash_equals("", "notempty"));
var_dump(hash_equals("notempty", ""));
var_dump(hash_equals("", ""));
