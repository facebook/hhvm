<?hh

<<__EntryPoint>>
function main_md5_upper_lower() :mixed{
  // MD5 of "foo" is acbd18db4cc2f85cedef654fccc4a4d8
  var_dump(HH\non_crypto_md5_upper("foo"));
  var_dump((0xacbd18db << 32) + 0x4cc2f85c);
  var_dump(HH\non_crypto_md5_lower("foo"));
  var_dump((0xedef654f << 32) + 0xccc4a4d8);

  // MD5 of "bar" is 37b51d194a7513e45b56f6524f2d51f2
  var_dump(HH\non_crypto_md5_upper("bar"));
  var_dump(0x37b51d194a7513e4);
  var_dump(HH\non_crypto_md5_lower("bar"));
  var_dump(0x5b56f6524f2d51f2);
}
