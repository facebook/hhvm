<?hh


function test_decryption($method) :mixed{
  $tests = openssl_get_cipher_tests($method);

  foreach ($tests as $idx => $test) {
    echo "TEST $idx\n";
    $pt = openssl_decrypt($test['ct'], $method, $test['key'], OPENSSL_RAW_DATA,
        $test['iv'], $test['tag'], $test['aad']);
    var_dump($test['pt'] === $pt);
  }

  // no IV
  var_dump(openssl_decrypt($test['ct'], $method, $test['key'], OPENSSL_RAW_DATA,
    '', $test['tag'], $test['aad']));
  // failed because no AAD
  var_dump(openssl_decrypt($test['ct'], $method, $test['key'], OPENSSL_RAW_DATA,
    $test['iv'], $test['tag']));
  // failed because wrong tag
  var_dump(openssl_decrypt($test['ct'], $method, $test['key'], OPENSSL_RAW_DATA,
    $test['iv'], str_repeat('x', 10), $test['aad']));
}
<<__EntryPoint>> function main(): void {
require_once __DIR__ . "/cipher_tests.inc";
// 128 bit CCM
test_decryption('aes-128-ccm');
// 256 bit CCM
test_decryption('aes-256-ccm');
}
