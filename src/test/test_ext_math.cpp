/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <test/test_ext_math.h>
#include <runtime/ext/ext_math.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtMath::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_pi);
  RUN_TEST(test_min);
  RUN_TEST(test_max);
  RUN_TEST(test_abs);
  RUN_TEST(test_is_finite);
  RUN_TEST(test_is_infinite);
  RUN_TEST(test_is_nan);
  RUN_TEST(test_ceil);
  RUN_TEST(test_floor);
  RUN_TEST(test_round);
  RUN_TEST(test_deg2rad);
  RUN_TEST(test_rad2deg);
  RUN_TEST(test_decbin);
  RUN_TEST(test_dechex);
  RUN_TEST(test_decoct);
  RUN_TEST(test_bindec);
  RUN_TEST(test_hexdec);
  RUN_TEST(test_octdec);
  RUN_TEST(test_base_convert);
  RUN_TEST(test_pow);
  RUN_TEST(test_exp);
  RUN_TEST(test_expm1);
  RUN_TEST(test_log10);
  RUN_TEST(test_log1p);
  RUN_TEST(test_log);
  RUN_TEST(test_cos);
  RUN_TEST(test_cosh);
  RUN_TEST(test_sin);
  RUN_TEST(test_sinh);
  RUN_TEST(test_tan);
  RUN_TEST(test_tanh);
  RUN_TEST(test_acos);
  RUN_TEST(test_acosh);
  RUN_TEST(test_asin);
  RUN_TEST(test_asinh);
  RUN_TEST(test_atan);
  RUN_TEST(test_atanh);
  RUN_TEST(test_atan2);
  RUN_TEST(test_hypot);
  RUN_TEST(test_fmod);
  RUN_TEST(test_sqrt);
  RUN_TEST(test_getrandmax);
  RUN_TEST(test_srand);
  RUN_TEST(test_rand);
  RUN_TEST(test_mt_getrandmax);
  RUN_TEST(test_mt_srand);
  RUN_TEST(test_mt_rand);
  RUN_TEST(test_lcg_value);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtMath::test_pi() {
  VC(f_pi(), k_M_PI);
  return Count(true);
}

bool TestExtMath::test_min() {
  VS(f_min(4, 2, CREATE_VECTOR4(3, 1, 6, 7)), 1);
  VS(f_min(0, CREATE_VECTOR3(2, 4, 5)), 2);
  VS(f_min(1, 0, CREATE_VECTOR1("hello")), 0);
  VS(f_min(1, "hello", CREATE_VECTOR1(0)), "hello");
  VS(f_min(1, "hello", CREATE_VECTOR1(-1)), -1);
  VS(f_min(1, CREATE_VECTOR3(2, 4, 8),
           CREATE_VECTOR1(CREATE_VECTOR3(2, 5, 1))),
     CREATE_VECTOR3(2, 4, 8));
  VS(f_min(1, "string", CREATE_VECTOR2(CREATE_VECTOR3(2, 5, 7), 42)),
     "string");
  VS(f_min(1, CREATE_MAP1(1, "1236150163")), "1236150163");
  return Count(true);
}

bool TestExtMath::test_max() {
  VS(f_max(4, 2, CREATE_VECTOR4(3, 1, 6, 7)), 7);
  VS(f_max(0, CREATE_VECTOR3(2, 4, 5)), 5);
  VS(f_max(1, 0, CREATE_VECTOR1("hello")), 0);
  VS(f_max(1, "hello", CREATE_VECTOR1(0)), "hello");
  VS(f_max(1, "hello", CREATE_VECTOR1(-1)), "hello");
  VS(f_max(1, CREATE_VECTOR3(2, 4, 8),
           CREATE_VECTOR1(CREATE_VECTOR3(2, 5, 1))),
     CREATE_VECTOR3(2, 5, 1));
  VS(f_max(1, "string", CREATE_VECTOR2(CREATE_VECTOR3(2, 5, 7), 42)),
     CREATE_VECTOR3(2, 5, 7));
  VS(f_max(1, CREATE_MAP1(1, "1236150163")), "1236150163");
  return Count(true);
}

bool TestExtMath::test_abs() {
  VS(f_abs(-4.2), 4.2);
  VS(f_abs(5), 5);
  VS(f_abs(-5), 5);
  VS(f_abs("-4.2"), 4.2);
  VS(f_abs("5"), 5);
  VS(f_abs("-5"), 5);
  return Count(true);
}

bool TestExtMath::test_is_finite() {
  VERIFY(f_is_finite(5));
  VERIFY(!f_is_finite(f_log(0)));
  return Count(true);
}

bool TestExtMath::test_is_infinite() {
  VERIFY(!f_is_infinite(5));
  VERIFY(f_is_infinite(f_log(0)));
  return Count(true);
}

bool TestExtMath::test_is_nan() {
  VERIFY(f_is_nan(f_acos(1.01)));
  return Count(true);
}

bool TestExtMath::test_ceil() {
  VS(f_ceil(4.3), 5.0);
  VS(f_ceil(9.999), 10.0);
  VS(f_ceil(-3.14), -3.0);
  return Count(true);
}

bool TestExtMath::test_floor() {
  VS(f_floor(4.3), 4.0);
  VS(f_floor(9.999), 9.0);
  VS(f_floor(-3.14), -4.0);
  return Count(true);
}

bool TestExtMath::test_round() {
  VS(f_round(3.4), 3.0);
  VS(f_round(3.5), 4.0);
  VS(f_round(3.6), 4.0);
  VS(f_round(3.6, 0), 4.0);
  VS(f_round(1.95583, 2), 1.96);
  VS(f_round(1241757, -3), 1242000.0);
  VS(f_round(5.045, 2), 5.05);
  VS(f_round(5.055, 2), 5.06);
  VS(f_round("3.4"), 3.0);
  VS(f_round("3.5"), 4.0);
  VS(f_round("3.6"), 4.0);
  VS(f_round("3.6", 0), 4.0);
  VS(f_round("1.95583", 2), 1.96);
  VS(f_round("1241757", -3), 1242000.0);
  VS(f_round("5.045", 2), 5.05);
  VS(f_round("5.055", 2), 5.06);
  return Count(true);
}

bool TestExtMath::test_deg2rad() {
  VC(f_deg2rad(45), k_M_PI_4);
  return Count(true);
}

bool TestExtMath::test_rad2deg() {
  VC(f_rad2deg(k_M_PI_4), 45);
  return Count(true);
}

bool TestExtMath::test_decbin() {
  VS(f_decbin(12), "1100");
  VS(f_decbin(26), "11010");
  return Count(true);
}

bool TestExtMath::test_dechex() {
  VS(f_dechex(10), "a");
  VS(f_dechex(47), "2f");
  return Count(true);
}

bool TestExtMath::test_decoct() {
  VS(f_decoct(15), "17");
  VS(f_decoct(264), "410");
  return Count(true);
}

bool TestExtMath::test_bindec() {
  VS(f_bindec("110011"), 51);
  VS(f_bindec("000110011"), 51);
  VS(f_bindec("111"), 7);
  return Count(true);
}

bool TestExtMath::test_hexdec() {
  VS(f_hexdec("See"), 238);
  VS(f_hexdec("ee"), 238);
  VS(f_hexdec("that"), 10);
  VS(f_hexdec("a0"), 160);
  return Count(true);
}

bool TestExtMath::test_octdec() {
  VS(f_octdec("77"), 63);
  VS(f_octdec(f_decoct(45)), 45);
  return Count(true);
}

bool TestExtMath::test_base_convert() {
  VS(f_base_convert("A37334", 16, 2), "101000110111001100110100");
  return Count(true);
}

bool TestExtMath::test_pow() {
  VC(f_pow(2, 8), 256);
  VC(f_pow(-1, 20), 1);
  VS(f_pow(0, 0), 1);
  VERIFY(f_pow(2, 32).isInteger());
  VC(f_pow("2", "8"), 256);
  VC(f_pow("-1", "20"), 1);
  VS(f_pow("0", "0"), 1);
  VERIFY(f_pow("2", "32").isInteger());
  return Count(true);
}

bool TestExtMath::test_exp() {
  VC(f_exp(12), 162754.791419);
  VC(f_exp(5.7), 298.86740096706);
  return Count(true);
}

bool TestExtMath::test_expm1() {
  VC(f_expm1(5.7), 297.8674);
  return Count(true);
}

bool TestExtMath::test_log10() {
  VS(f_log10(10), 1.0);
  VS(f_log10(100), 2.0);
  return Count(true);
}

bool TestExtMath::test_log1p() {
  VC(f_log1p(9), 2.302585092994);
  VC(f_log1p(99), 4.6051701859881);
  return Count(true);
}

bool TestExtMath::test_log() {
  VC(f_log(8), 2.0794415416798);
  VS(f_log(8, 2), 3.0);
  return Count(true);
}

bool TestExtMath::test_cos() {
  VC(f_cos(k_M_PI), -1);
  return Count(true);
}

bool TestExtMath::test_cosh() {
  VC(f_cosh(1.23), 1.8567610569853);
  return Count(true);
}

bool TestExtMath::test_sin() {
  VC(f_sin(f_deg2rad(90)), 1);
  return Count(true);
}

bool TestExtMath::test_sinh() {
  VC(f_sinh(1.23), 1.5644684793044);
  return Count(true);
}

bool TestExtMath::test_tan() {
  VC(f_tan(f_deg2rad(45)), 1);
  return Count(true);
}

bool TestExtMath::test_tanh() {
  VC(f_tanh(1.23), 0.84257932565893);
  return Count(true);
}

bool TestExtMath::test_acos() {
  VC(f_acos(-1), k_M_PI);
  return Count(true);
}

bool TestExtMath::test_acosh() {
  VC(f_acosh(1.8567610569853), 1.23);
  return Count(true);
}

bool TestExtMath::test_asin() {
  VC(f_asin(1), f_deg2rad(90));
  return Count(true);
}

bool TestExtMath::test_asinh() {
  VC(f_asinh(1.5644684793044), 1.23);
  return Count(true);
}

bool TestExtMath::test_atan() {
  VC(f_atan(1), f_deg2rad(45));
  return Count(true);
}

bool TestExtMath::test_atanh() {
  VC(f_atanh(0.84257932565893), 1.23);
  return Count(true);
}

bool TestExtMath::test_atan2() {
  VC(f_atan2(3, 4), 0.64350110879328);
  return Count(true);
}

bool TestExtMath::test_hypot() {
  VS(f_hypot(3, 4), 5.0);
  return Count(true);
}

bool TestExtMath::test_fmod() {
  VS(f_fmod(5.7, 1.3), 0.5);
  return Count(true);
}

bool TestExtMath::test_sqrt() {
  VS(f_sqrt(9), 3.0);
  return Count(true);
}

bool TestExtMath::test_getrandmax() {
  VS(f_getrandmax(), 2147483647);
  return Count(true);
}

bool TestExtMath::test_srand() {
  f_srand();
  f_srand(0);
  f_srand(1);
  return Count(true);
}

bool TestExtMath::test_rand() {
  f_rand();
  VERIFY(f_rand(5, 15) >= 5);
  VERIFY(f_rand(5, 15) <= 15);

  int64 n = f_rand(10000000000, 19999999999);
  VERIFY(n >= 10000000000);
  VERIFY(n <= 19999999999);

  return Count(true);
}

bool TestExtMath::test_mt_getrandmax() {
  VS(f_mt_getrandmax(), 2147483647);
  return Count(true);
}

bool TestExtMath::test_mt_srand() {
  f_mt_srand();
  f_mt_srand(0);
  f_mt_srand(1);
  return Count(true);
}

bool TestExtMath::test_mt_rand() {
  f_mt_rand();
  VERIFY(f_mt_rand(5, 15) >= 5);
  VERIFY(f_mt_rand(5, 15) <= 15);
  return Count(true);
}

bool TestExtMath::test_lcg_value() {
  VERIFY(f_lcg_value() >= 0);
  VERIFY(f_lcg_value() <= 1);
  return Count(true);
}
