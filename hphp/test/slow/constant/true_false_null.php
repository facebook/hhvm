<?php

const NOT_REALLY_TRUE = false;
const ANTI_NULL = true;
const FAKE_FALSE = 1;

use const NOT_REALLY_TRUE as true;
use const FAKE_FALSE as false;
use const ANTI_NULL as null;

var_dump(true);
var_dump(false);
var_dump(null);
