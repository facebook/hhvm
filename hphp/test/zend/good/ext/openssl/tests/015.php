<?hh
class test {
        function __toString() :mixed{
                return "test";
        }
}
<<__EntryPoint>>
function entrypoint_015(): void {
  $data = "Testing openssl_public_encrypt()";
  $privkey = "file://" . dirname(__FILE__) . "/private.key";
  $pubkey = "file://" . dirname(__FILE__) . "/public.key";
  $wrong = "wrong";
  $obj = new test;

  $encrypted = null;
  $encrypted_bad = null;
  var_dump(openssl_public_encrypt($data, inout $encrypted, $pubkey));
  var_dump(openssl_public_encrypt($data, inout $encrypted_bad, $privkey));
  var_dump(openssl_public_encrypt($data, inout $encrypted_bad, $wrong));
  var_dump(openssl_public_encrypt($data, inout $encrypted_bad, $obj));
  $output = null;
  openssl_private_decrypt($encrypted, inout $output, $privkey);
  var_dump($output);
}
