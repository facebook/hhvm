<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

$td = mcrypt_module_open("rijndael-256", "", "ofb", "");
$iv = mcrypt_create_iv(mcrypt_enc_get_iv_size($td),
                                MCRYPT_DEV_RANDOM);
$ks = mcrypt_enc_get_key_size($td);
$key = substr(md5("very secret key"), 0, $ks);
mcrypt_generic_init($td, $key, $iv);
$encrypted = mcrypt_generic($td, "This is very important data");
VERIFY($encrypted !== "This is very important data");
mcrypt_generic_deinit($td);
mcrypt_generic_init($td, $key, $iv);
$decrypted = mdecrypt_generic($td, $encrypted);
mcrypt_generic_end($td);
mcrypt_module_close($td);

VS($decrypted, "This is very important data");

VERIFY(in_array("blowfish", mcrypt_list_algorithms()));
VERIFY(in_array("cbc", mcrypt_list_modes()));
VS(mcrypt_module_get_algo_block_size("blowfish"), 8);
VS(mcrypt_module_get_algo_key_size("blowfish"), 56);

VS(mcrypt_module_get_supported_key_sizes("blowfish"), array());
VS(mcrypt_module_get_supported_key_sizes("twofish"),
   array(16, 24, 32));

VS(mcrypt_module_is_block_algorithm_mode("cbc"), true);
VS(mcrypt_module_is_block_algorithm("blowfish"), true);
VS(mcrypt_module_is_block_mode("cbc"), true);
VS(mcrypt_module_self_test(MCRYPT_RIJNDAEL_128), true);
VS(mcrypt_module_self_test("bogus"), false);

$text = "boggles the inivisble monkey will rule the world";
$key = "very secret key";
$iv_size = mcrypt_get_iv_size(MCRYPT_XTEA, MCRYPT_MODE_ECB);
$iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
$enc = mcrypt_encrypt(MCRYPT_XTEA, $key, $text, MCRYPT_MODE_ECB,
                               $iv);
VS(bin2hex($enc), "f522c62002fa16129c8576bcddc6dd0f7ea81991103ba42962d94c8bfff3ee660d53b187d7e989540abf5a729c2f7baf");
$crypttext = mcrypt_decrypt(MCRYPT_XTEA, $key, $enc,
                                     MCRYPT_MODE_ECB, $iv);
VS($crypttext, $text);

//////////////////////////////////////////////////////////////////////

$key = "123456789012345678901234567890123456789012345678901234567890";
$CC = "4007000000027";
$encrypted =
  mcrypt_cbc(MCRYPT_RIJNDAEL_128, substr($key,0,32),
               $CC, MCRYPT_ENCRYPT, substr($key,32,16));
$decrypted =
  mcrypt_cbc(MCRYPT_RIJNDAEL_128, substr($key,0,32),
               $encrypted, MCRYPT_DECRYPT, substr($key,32,16));
VERIFY($encrypted !== $decrypted);
VS(trim((string)$decrypted), $CC);

//////////////////////////////////////////////////////////////////////

$key = "123456789012345678901234567890123456789012345678901234567890";
$CC = "4007000000027";
$encrypted =
  mcrypt_cfb(MCRYPT_RIJNDAEL_128, substr($key,0,32),
               $CC, MCRYPT_ENCRYPT, substr($key,32,16));
$decrypted =
  mcrypt_cfb(MCRYPT_RIJNDAEL_128, substr($key,0,32),
               $encrypted, MCRYPT_DECRYPT, substr($key,32,16));
VERIFY($encrypted !== $decrypted);
VS(trim((string)$decrypted), $CC);

//////////////////////////////////////////////////////////////////////

$key = "123456789012345678901234567890123456789012345678901234567890";
$CC = "4007000000027";
$encrypted =
  mcrypt_ecb(MCRYPT_RIJNDAEL_128, substr($key,0,32),
               $CC, MCRYPT_ENCRYPT, substr($key,32,16));
$decrypted =
  mcrypt_ecb(MCRYPT_RIJNDAEL_128, substr($key,0,32),
               $encrypted, MCRYPT_DECRYPT, substr($key,32,16));
VERIFY($encrypted !== $decrypted);
VS(trim((string)$decrypted), $CC);

//////////////////////////////////////////////////////////////////////

$key = "123456789012345678901234567890123456789012345678901234567890";
$CC = "4007000000027";
$encrypted =
  mcrypt_ofb(MCRYPT_RIJNDAEL_128, substr($key,0,32),
               $CC, MCRYPT_ENCRYPT, substr($key,32,16));
$decrypted =
  mcrypt_ofb(MCRYPT_RIJNDAEL_128, substr($key,0,32),
               $encrypted, MCRYPT_DECRYPT, substr($key,32,16));
VERIFY($encrypted !== $decrypted);
VS($decrypted, $CC);

//////////////////////////////////////////////////////////////////////

VS(mcrypt_get_block_size("tripledes", "ecb"), 8);
VS(mcrypt_get_cipher_name(MCRYPT_TRIPLEDES), "3DES");
VS(mcrypt_get_iv_size(MCRYPT_CAST_256, MCRYPT_MODE_CFB), 16);
VS(mcrypt_get_iv_size("des", "ecb"), 8);
VS(mcrypt_get_key_size("tripledes", "ecb"), 24);

$td = mcrypt_module_open("cast-256", "", "cfb", "");
VS(mcrypt_enc_get_algorithms_name($td), "CAST-256");

$td = mcrypt_module_open("tripledes", "", "ecb", "");
VS(mcrypt_enc_get_block_size($td), 8);

$td = mcrypt_module_open("cast-256", "", "cfb", "");
VS(mcrypt_enc_get_iv_size($td), 16);

$td = mcrypt_module_open("tripledes", "", "ecb", "");
VS(mcrypt_enc_get_key_size($td), 24);

$td = mcrypt_module_open("cast-256", "", "cfb", "");
VS(mcrypt_enc_get_modes_name($td), "CFB");

$td = mcrypt_module_open("rijndael-256", "", "ecb", "");
VS(mcrypt_enc_get_supported_key_sizes($td),
   array(16, 24, 32));

$td = mcrypt_module_open("tripledes", "", "ecb", "");
VS(mcrypt_enc_is_block_algorithm_mode($td), true);

$td = mcrypt_module_open("tripledes", "", "ecb", "");
VS(mcrypt_enc_is_block_algorithm($td), true);

$td = mcrypt_module_open("tripledes", "", "ecb", "");
VS(mcrypt_enc_is_block_mode($td), true);

$td = mcrypt_module_open("tripledes", "", "ecb", "");
VS(mcrypt_enc_self_test($td), 0);
