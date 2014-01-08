<?php

// Arbitrary - must not be UTC, and better if it's not where you are, or where
// tests are run
date_default_timezone_set('Asia/Jerusalem');

// All the standard formats including offsets
$format = "c\nO\nZ\nr";
var_dump((new DateTime('2014-01-01 00:00'))->format($format));
var_dump((new DateTime('@0'))->format($format));
