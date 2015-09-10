<?php

/*
 * Cause errors (like division by zero) to raise an exception instead of
 * emitting warnings.
 */
function errorHandler($errno, $errstr, $errfile, $errline) {
  throw new Exception;
}
set_error_handler('errorHandler');

print("\n# is_numeric tests\n");
printf("#%9s, %5s, %7s, %7s\n",
  'input', 'isnum', 'int', 'float');

function do_is_numeric($x) {
  $numeric = is_numeric($x) ? "true" : "false";
  $int_cast = var_export((int) $x, true);
  $float_cast = var_export((float) $x, true);
  printf("%10s, %5s, %7s, %7s\n",
    $x, $numeric, $int_cast, $float_cast);
}

do_is_numeric("100");
do_is_numeric("0100");
do_is_numeric("0x100");
do_is_numeric("0x0100");
do_is_numeric("-100");
do_is_numeric("-0100");
do_is_numeric("-0x100");
do_is_numeric("-0x0100");
do_is_numeric("+100");
do_is_numeric("+0100");
do_is_numeric("+0x100");
do_is_numeric("+0x0100");

do_is_numeric("100.5");
do_is_numeric("0100.5");
do_is_numeric("0x100.5");
do_is_numeric("0x0100.5");
do_is_numeric("-100.5");
do_is_numeric("-0100.5");
do_is_numeric("-0x100.5");
do_is_numeric("-0x0100.5");
do_is_numeric("+100.5");
do_is_numeric("+0100.5");
do_is_numeric("+0x100.5");
do_is_numeric("+0x0100.5");

print("\n# implicit cast on operators (out is var_export)\n");
printf("#%12s, %3s, %10s, %10s\n",
  'in_num', 'op', 'in_str', 'out');

abstract class Operator {
  const COMP_EQ = '==';
  const PLUS = '+';
  const MINUS = '-';
  const TIMES = '*';
  const DIVIDE = '/';
  const MOD = '%';
  const POWER = '**';
  const PLUSPLUS = '++';
  const MINMIN = '--';
}

function do_op($x, $op, $y) {
  $res = '??';

  try {
    switch($op) {
    case Operator::COMP_EQ:
      $res = ($x == $y) ? 'true' : 'false';
      break;
    case Operator::PLUS:
      $res = var_export($x + $y, true);
      break;
    case Operator::MINUS:
      $res = var_export($x - $y, true);
      break;
    case Operator::TIMES:
      $res = var_export($x * $y, true);
      break;
    case Operator::DIVIDE:
      $res = var_export($x/$y, true);
      break;
    case Operator::MOD:
      $res = var_export($x % $y, true);
      break;
    case Operator::POWER:
      $res = var_export($x ** $y, true);
      break;
    case Operator::PLUSPLUS:
      $res = $x;
      $res++;
      $res = var_export($res, true);
      break;
    case Operator::MINMIN:
      $res = $x;
      $res--;
      $res = var_export($res, true);
      break;
    }
  } catch (Exception $e) {
    $res = 'ERR';
  }

  printf("%10s, %3s, %10s, %10s\n",
    var_export($x, true), $op, var_export($y, true), $res);
}

do_op(100, Operator::COMP_EQ, "100");
do_op(100, Operator::COMP_EQ, "0100");
do_op(0x100, Operator::COMP_EQ, "0x100");
do_op(0x100, Operator::COMP_EQ, "0x0100");
do_op(-100, Operator::COMP_EQ, "-100");
do_op(-100, Operator::COMP_EQ, "-0100");
do_op(0x100 * -1, Operator::COMP_EQ, "-0x100");
do_op(0x100 * -1, Operator::COMP_EQ, "-0x0100");
do_op(100, Operator::COMP_EQ, "+100");
do_op(100, Operator::COMP_EQ, "+0100");
do_op(0x100, Operator::COMP_EQ, "+0x100");
do_op(0x100, Operator::COMP_EQ, "+0x0100");

do_op(100.5, Operator::COMP_EQ, "100.5");
do_op(100.5, Operator::COMP_EQ, "0100.5");
do_op(0x100 + 5/16.0, Operator::COMP_EQ, "0x100.5");
do_op(0x100 + 5/16.0, Operator::COMP_EQ, "0x0100.5");
do_op(-100.5, Operator::COMP_EQ, "-100.5");
do_op(-100.5, Operator::COMP_EQ, "-0100.5");
do_op(100.5, Operator::COMP_EQ, "+100.5");
do_op(100.5, Operator::COMP_EQ, "+0100.5");
do_op(0x100 + 5/16.0, Operator::COMP_EQ, "+0x100.5");
do_op(0x100 + 5/16.0, Operator::COMP_EQ, "+0x0100.5");

/*
 * Now just a meaningful subset of the above, since lots of them are not number
 * in any context on any version, and we've proven that in a couple ways
 * already.
 */

function do_all_op($op) {
  do_op(10, $op, "10");
  do_op(10, $op, "010");
  do_op(0x10, $op, "0x10");
  do_op(0x10, $op, "0x010");
  do_op(-10, $op, "-10");
  do_op(-10, $op, "-010");
  do_op(10, $op, "+10");
  do_op(10, $op, "+010");

  do_op(10.5, $op, "10.5");
  do_op(10.5, $op, "010.5");
  do_op(-10.5, $op, "-10.5");
  do_op(-10.5, $op, "-010.5");
  do_op(10.5, $op, "+10.5");
  do_op(10.5, $op, "+010.5");
}

do_all_op(Operator::PLUS);
do_all_op(Operator::MINUS);
do_all_op(Operator::TIMES);

/*
 * using reasonable numbers for these operators. We're pretty close to wasting
 * time with test cases at this point.
 */

do_op(11, Operator::DIVIDE, "2");
do_op(0x21, Operator::DIVIDE, "0x10");
do_op("11", Operator::DIVIDE, 2);
do_op("0x21", Operator::DIVIDE, 0x10);
do_op(11, Operator::MOD, "2");
do_op(0x21, Operator::MOD, "0x10");
do_op("11", Operator::MOD, 2);
do_op("0x21", Operator::MOD, 0x10);

do_op(11, Operator::POWER, "2");
do_op(0x21, Operator::POWER, "0x10");
do_op("11", Operator::POWER, 2);
do_op("0x21", Operator::POWER, 0x10);

print("# Warning: ++ and -- do odd things to strings.\n");
print("# ++ increments the final character by 1.\n");
print("# -- seems to do nothing, leaving the string as-is.\n");
do_op("11", Operator::PLUSPLUS, 'ign');
do_op("0x21", Operator::PLUSPLUS, 'ign');
do_op("11", Operator::MINMIN, 'ign');
do_op("0x21", Operator::MINMIN, 'ign');
