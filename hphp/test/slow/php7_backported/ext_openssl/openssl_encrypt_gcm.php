<?hh

function test_encryption($method) :mixed{
  $tests = openssl_get_cipher_tests($method);

  foreach ($tests as $idx => $test) {
    echo "TEST $idx\n";
    $tag = null;
    $ct = openssl_encrypt_with_tag($test['pt'], $method, $test['key'], OPENSSL_RAW_DATA,
        $test['iv'], inout $tag, $test['aad'], strlen($test['tag']));
    var_dump($test['ct'] === $ct);
    var_dump($test['tag'] === $tag);
  }

  // Empty IV error
  var_dump(openssl_encrypt_with_tag('data', $method, 'password', 0, '', inout $tag, ''));

  // Failing to retrieve tag (max is 16 bytes)
  var_dump(openssl_encrypt_with_tag('data', $method, 'password', 0, str_repeat('x', 32), inout $tag, '', 20));
}
<<__EntryPoint>> function main(): void {
require_once __DIR__ . "/cipher_tests.inc";
// 128 bit GCM
test_encryption('aes-128-gcm');
// 192 bit GCM
test_encryption('aes-192-gcm');
// 256 bit GCM
test_encryption('aes-256-gcm');
}
