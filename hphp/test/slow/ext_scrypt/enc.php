<?hh


<<__EntryPoint>>
function main_enc() :mixed{
echo scrypt_enc("pleaseletmein", "SodiumChloride", 14, 8, 1) . "\n";
echo scrypt_enc("password", "toomuchsalt", 14, 8 ,1) . "\n";
echo scrypt_enc("supersecure", "notthepassword", 14, 8, 1) . "\n";
echo scrypt_enc("thisisntthepasswordyouarelookingfor", "butthisis", 14, 8 , 1)
  . "\n";
echo scrypt_enc("password123", "1234567", 14, 8, 1) . "\n";
echo scrypt_enc(
  "\u{3061}\u{3087}\u{4A63}\u{3074}\u{3083}\u{3044}\u{82E8} \u{5C24}",
  "12\u{30AF}asdf", 14, 8, 1) . "\n";
echo scrypt_enc("(\u{256F}\u{00B0}\u{25A1}\u{00B0}\u{FF09}\u{256F}\u{FE35} " .
                "\u{253B}\u{2501}\u{253B}", "tableflip", 15, 8, 1) . "\n";
echo scrypt_enc("it works", "really a pepper", 20, 8, 1). "\n";

if(FALSE===scrypt_enc("it works", "really a pepper", 1048576, 8, 1)) {
  echo "Failure === test worked\n";
}
}
