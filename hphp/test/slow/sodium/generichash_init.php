<?hh

<<__EntryPoint>>
function main() :mixed{
  $key =
    hex2bin('36be2998c85757e98c1abf3687c8db3a849a393701c05454023d9aba1096fd47');
  $y = sodium_crypto_generichash_init($key, 64);
  var_dump(bin2hex($y));
}
