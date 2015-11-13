<?php

assert_options(ASSERT_WARNING, 1);
assert_options(ASSERT_ACTIVE, 1);

var_dump(ini_get('assert.active'));
ini_set('assert.active', 0);
var_dump(ini_get('assert.active'));
var_dump(assert_options(ASSERT_ACTIVE));
assert(false);
