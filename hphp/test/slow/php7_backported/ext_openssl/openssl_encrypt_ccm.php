<?hh
require_once __DIR__ . "/cipher_tests.inc";

function test_encryption($method) {
  $tests = openssl_get_cipher_tests($method);

  foreach ($tests as $idx => $test) {
    echo "TEST $idx\n";
    $ct = openssl_encrypt_with_tag($test['pt'], $method, $test['key'], OPENSSL_RAW_DATA,
        $test['iv'], &$tag, $test['aad'], strlen($test['tag']));
    var_dump($test['ct'] === $ct);
    var_dump($test['tag'] === $tag);
  }

  // Empty IV error
  var_dump(openssl_encrypt_with_tag('data', $method, 'password', 0, '', &$tag, ''));

  // Test setting different IV length and unlimeted tag
  var_dump(openssl_encrypt_with_tag('data', $method, 'password', 0, str_repeat('x', 10), &$tag, '', 1024));
}
<<__EntryPoint>> function main(): void {
// 128 bit CCM
test_encryption('aes-128-ccm');
// 256 bit CCM
test_encryption('aes-256-ccm');
}
