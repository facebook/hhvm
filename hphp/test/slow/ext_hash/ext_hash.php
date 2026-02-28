<?hh

// HPHP has has many version of these functions over time
// Faceobok has implementation differences
function is_facebook() :mixed{
  return extension_loaded("facebook");
}

function brown_fox() :mixed{
  echo "==== brown_fox ====\n";
  $data = "The quick brown fox jumped over the lazy dog.";
  var_dump(hash("md2",        $data));
  var_dump(hash("md4",        $data));
  var_dump(hash("md5",        $data));
  var_dump(hash("sha1",       $data));
  var_dump(hash("sha256",     $data));
  var_dump(hash("sha384",     $data));
  var_dump(hash("sha512",     $data));
  var_dump(hash("ripemd128",  $data));
  var_dump(hash("ripemd160",  $data));
  var_dump(hash("ripemd256",  $data));
  var_dump(hash("ripemd320",  $data));
  var_dump(hash("whirlpool",  $data));
  var_dump(hash("tiger128,3", $data));
  var_dump(hash("tiger160,3", $data));
  var_dump(hash("tiger192,3", $data));
  var_dump(hash("tiger128,4", $data));
  var_dump(hash("tiger160,4", $data));
  var_dump(hash("tiger192,4", $data));
  var_dump(hash("snefru",     $data));
  var_dump(hash("gost",       $data));
  var_dump(hash("crc32",      $data));
  var_dump(hash("haval128,3", $data));
  var_dump(hash("haval160,3", $data));
  var_dump(hash("haval192,3", $data));
  var_dump(hash("haval224,3", $data));
  var_dump(hash("haval256,3", $data));
  var_dump(hash("haval128,4", $data));
  var_dump(hash("haval160,4", $data));
  var_dump(hash("haval192,4", $data));
  var_dump(hash("haval224,4", $data));
  var_dump(hash("haval256,4", $data));
  var_dump(hash("haval128,5", $data));
  var_dump(hash("haval160,5", $data));
  var_dump(hash("haval192,5", $data));
  var_dump(hash("haval224,5", $data));
  var_dump(hash("haval256,5", $data));
  var_dump(hash("adler32",    $data));
  var_dump(hash("crc32b",     $data));
  var_dump(hash("crc32c",     $data));
  var_dump(hash("blake3",     $data));

  if (is_facebook()) {
    var_dump(hash("keyed-blake3", $data));
    var_dump(
      hash("tiger128,3-fb", $data) === '9370512795923aaeeb76fe3d8ea7433e' &&
      hash("adler32-fb", $data) === '5e10f17b'
    );
  } else {
    var_dump(true);
  }
}

function test_hash_init() :mixed{
  echo "==== test_hash_init ====\n";
  $ctx = hash_init("md5");
  hash_update($ctx, "The quick brown fox ");
  hash_update($ctx, "jumped over the lazy dog.");
  var_dump(hash_final($ctx));
}

function test_hash_file() :mixed{
  echo "==== test_hash_file ====\n";
  var_dump(hash_file('md5', __DIR__.'/test_file.txt'));
  var_dump(hash_hmac_file("md5", __DIR__.'/test_file.txt', "secret"));
}

function test_hash_hmac() :mixed{
  echo "==== test_hash_hmac ====\n";
  $data = "the quick brown fox jumped over the lazy dog.";
  var_dump(hash_hmac("md5", $data, "secret"));
  var_dump(hash_hmac("md5", $data, ""));
}

function test_furchash() :mixed{
  echo "==== test_furchash ====\n";
  if (is_facebook()) {
    var_dump(furchash_hphp_ext("15minutesoffame", 15, 86) === 25);
  } else {
    var_dump(furchash_hphp_ext("15minutesoffame", 15, 86) === 85);
  }
}

<<__EntryPoint>> function main_ext_hash() :mixed{
  brown_fox();
  test_hash_init();
  test_hash_file();
  test_furchash();
  test_hash_hmac();
}
