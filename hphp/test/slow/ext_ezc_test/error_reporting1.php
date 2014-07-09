<?php

/*
 * Call the wrapper around zend's EG(error_reporting) macro
 * to test that EG(error_reporting) in both lval and rval context
 * works plausibly.
 */
$val = ezc_get_error_reporting();
printf("initial 0x%08x\n", $val);

/*
 * Call the existing PHP builtin function error_reporting to set the
 * value behind EG(error_reporting), as visible through the test
 * function ezc_get_error_reporting().
 */
error_reporting(0x1);
$val = ezc_get_error_reporting();
printf("0x1 0x%08x\n", $val);

error_reporting(E_ERROR);
$val = ezc_get_error_reporting();
printf("0x1 0x%08x\n", $val);

error_reporting(0x3);
$val = ezc_get_error_reporting();
printf("0x3 0x%08x\n", $val);

error_reporting(E_ERROR|E_WARNING);
$val = ezc_get_error_reporting();
printf("0x3 0x%08x\n", $val);

/*
 * Tests that call ezc_set_error_reporting to manipulate
 * EG(error_reporting) in lval context.
 */
printf("----\n");
error_reporting(E_ERROR);
$val_a = ezc_set_error_reporting(E_ERROR|E_WARNING);
$val_b = error_reporting();
printf("0x1 0x%08x\n", $val_a);
printf("0x3 0x%08x\n", $val_b);

printf("----\n");
error_reporting(0x1eadbeef);
$val_a = ezc_set_error_reporting(0x0badf00d);
$val_b = ezc_set_error_reporting(E_ERROR|E_WARNING);
$val_c = error_reporting();
printf("0x1eadbeef 0x%08x\n", $val_a);
printf("0x0badf00d 0x%08x\n", $val_b);
printf("0x3 0x%08x\n", $val_c);
