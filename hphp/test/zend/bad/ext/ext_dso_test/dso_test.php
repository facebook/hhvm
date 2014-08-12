<?php

var_dump(17);
var_dump(dso_test_world());

var_dump(dso_test_long());
var_dump(dso_test_long());
var_dump(dso_test_long());

var_dump(dso_test_double());
var_dump(dso_test_bool());
var_dump(dso_test_null());

ini_set("dso_test.direction", 0);
var_dump(dso_test_long());
var_dump(dso_test_long());
var_dump(dso_test_long());
