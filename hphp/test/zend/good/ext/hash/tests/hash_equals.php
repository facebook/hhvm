<?php
var_dump(hash_equals("same", "same"));
var_dump(hash_equals("not1same", "not2same"));
var_dump(hash_equals("short", "longer"));
var_dump(hash_equals("longer", "short"));
var_dump(hash_equals("", "notempty"));
var_dump(hash_equals("notempty", ""));
var_dump(hash_equals("", ""));
var_dump(hash_equals(123, "NaN"));
var_dump(hash_equals("NaN", 123));
var_dump(hash_equals(123, 123));
var_dump(hash_equals(null, ""));
var_dump(hash_equals(null, 123));
var_dump(hash_equals(null, null));
