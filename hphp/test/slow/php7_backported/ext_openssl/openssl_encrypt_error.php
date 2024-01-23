<?hh

<<__EntryPoint>>
function main(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    if (HH\Lib\Str\starts_with($errstr, "Argument")) {
      throw new Exception($errstr);
    }
    return false;
  });

  $data = "openssl_encrypt() tests";
  $method = "AES-128-CBC";
  $password = "openssl";
  $iv = str_repeat("\0", openssl_cipher_iv_length($method));
  $wrong = "wrong";
  $object = new stdClass;
  $arr = vec[1];

  // wrong paramters tests
  var_dump(openssl_encrypt($data, $wrong, $password));
  try { var_dump(openssl_encrypt($object, $method, $password)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(openssl_encrypt($data, $object, $password)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(openssl_encrypt($data, $method, $object)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(openssl_encrypt($arr, $method, $object)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(openssl_encrypt($data, $arr, $object)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(openssl_encrypt($data, $method, $arr)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  // invalid using of an authentication tag
  var_dump(openssl_encrypt_with_tag($data, $method, $password, 0, $iv, inout $wrong));
}
