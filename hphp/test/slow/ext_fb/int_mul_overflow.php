<?hh

<<__EntryPoint>>
function main_int_mul_overflow() :mixed{
  // ======================================================
  // int_mul_overflow
  // ======================================================
  // Just short of overflow -> 0
  var_dump(HH\int_mul_overflow(0xffffffff, 0x100000001));
  // Just barely overflow -> 1
  var_dump(HH\int_mul_overflow(0xffffffff, 0x100000002));
  // PHP_INT_MAX acts just short of 1/2
  var_dump(HH\int_mul_overflow(PHP_INT_MAX, 42));
  var_dump(HH\int_mul_overflow(42, PHP_INT_MAX));
  var_dump(HH\int_mul_overflow(12345678, PHP_INT_MAX));
  var_dump(HH\int_mul_overflow(1234567891011121314, PHP_INT_MAX));
  var_dump(HH\int_mul_overflow(-1, PHP_INT_MAX));
  // PHP_INT_MIN acts like 1/2
  var_dump(HH\int_mul_overflow(PHP_INT_MIN, 42));
  var_dump(HH\int_mul_overflow(42, PHP_INT_MIN));
  var_dump(HH\int_mul_overflow(12345678, PHP_INT_MIN));
  var_dump(HH\int_mul_overflow(1234567891011121314, PHP_INT_MIN));
  var_dump(HH\int_mul_overflow(-1, PHP_INT_MIN));
  // (unsigned)-1 acts just short of 1
  var_dump(HH\int_mul_overflow(-1, 42));
  var_dump(HH\int_mul_overflow(42, -1));
  var_dump(HH\int_mul_overflow(12345678, -1));
  var_dump(HH\int_mul_overflow(1234567891011121314, -1));
  var_dump(HH\int_mul_overflow(-1, -1));
  // ======================================================
  // int_mul_add_overflow
  // ======================================================
  // Just short of overflow -> 0
  var_dump(HH\int_mul_add_overflow(0xffffffff, 0x100000001, 0));
  // Just barely overflow -> 1
  var_dump(HH\int_mul_add_overflow(0xffffffff, 0x100000001, 1));
  // etc
  var_dump(HH\int_mul_add_overflow(PHP_INT_MAX, 242, 0));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MAX, 242, 241));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MAX, 242, 242));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MAX, 242, PHP_INT_MAX));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MAX, 242, -1));
  // etc
  var_dump(HH\int_mul_add_overflow(PHP_INT_MAX, 2044, 0));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MAX, 2044, 2043));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MAX, 2044, 2044));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MAX, 2044, PHP_INT_MAX));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MAX, 2044, -1));
  // etc
  var_dump(HH\int_mul_add_overflow(PHP_INT_MIN, 41, PHP_INT_MAX));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MIN, 41, PHP_INT_MIN));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MIN, 42, 0));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MIN, 42, -1));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MIN, 43, PHP_INT_MAX));
  var_dump(HH\int_mul_add_overflow(PHP_INT_MIN, 43, PHP_INT_MIN));
}
