<?php

assert_options(ASSERT_ACTIVE, 0);
assert_options(ASSERT_WARNING, 0);
var_dump(assert(false));
var_dump(assert_options(ASSERT_ACTIVE, 0));
var_dump(assert_options(ASSERT_WARNING, 1));
var_dump(assert(false));
var_dump(assert_options(ASSERT_ACTIVE, 0));
var_dump(assert_options(ASSERT_WARNING, 2));
var_dump(assert(false));
var_dump(assert_options(ASSERT_ACTIVE, 1));
var_dump(assert_options(ASSERT_WARNING, 0));
var_dump(assert(false));
var_dump(assert_options(ASSERT_ACTIVE, 2));
var_dump(assert_options(ASSERT_WARNING, 0));
var_dump(assert(false));
