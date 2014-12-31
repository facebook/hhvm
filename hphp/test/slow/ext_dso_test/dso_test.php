<?php

var_dump(17);
var_dump(dso_test_world());

var_dump(dso_test_long());
var_dump(dso_test_long());
var_dump(dso_test_long());

var_dump(dso_test_double());
var_dump(dso_test_bool());
var_dump(dso_test_null());

/*
 * Alas, due to a bug in the way that ezc DSOs are loaded and their
 * configurations processed, setting dso_test.direction does not
 * yet work; the value remains sticky at 1, and every access to
 * dso_test_long post-increments the value.
 */
ini_set("dso_test.direction", 0);
var_dump(dso_test_long());
var_dump(dso_test_long());
var_dump(dso_test_long());
