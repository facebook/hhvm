<?hh
class test {
        function __toString() :mixed{
                return "test";
        }
}
<<__EntryPoint>>
function entrypoint_014(): void {
  $data = "Testing openssl_private_encrypt()";
  $privkey = "file://" . dirname(__FILE__) . "/private.key";
  $pubkey = "file://" . dirname(__FILE__) . "/public.key";
  $wrong = "wrong";
  $obj = new test;

  $encrypted = null;
  $encrypted_bad = null;
  var_dump(openssl_private_encrypt($data, inout $encrypted, $privkey));
  var_dump(openssl_private_encrypt($data, inout $encrypted_bad, $pubkey));
  var_dump(openssl_private_encrypt($data, inout $encrypted_bad, $wrong));
  var_dump(openssl_private_encrypt($data, inout $encrypted_bad, $obj));
  try { var_dump(openssl_private_encrypt($obj, inout $encrypted_bad, $privkey)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $output = null;
  openssl_public_decrypt($encrypted, inout $output, $pubkey);
  var_dump($output);
}
