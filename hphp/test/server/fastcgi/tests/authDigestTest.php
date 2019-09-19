<?hh

require_once('test_base.inc');

function BadAuthDigestTestController($serverPort) {
  $args = array('Authorization' => 'Digest "username="admin", ' .
    'realm="Restricted area", nonce="564a12f5c065e", ' .
    'uri="/test_auth_digest.php", cnonce="MjIyMTg2", nc=00000001, ' .
    'qop="auth", response="6dfbea52fbf13016476c1879e6436004", ' .
    'opaque="cdce8a5c95a1427d74df7acbf41c9ce0"');
  var_dump(request('localhost', $serverPort, "test_auth_digest.php",
                  [], [], $args));
}

function GoodAuthDigestTestController($serverPort) {
  $args = array('Authorization' => 'Digest username="admin", ' .
    'realm="Restricted area", nonce="564a12611dae8", ' .
    'uri="/test_auth_digest.php", cnonce="MjIyMTg1", nc=00000001, ' .
    'qop="auth", response="e544aaed06917adea3e5c74dd49f0e32", ' .
    'opaque="cdce8a5c95a1427d74df7acbf41c9ce0"');
  var_dump(request('localhost', $serverPort, "test_auth_digest.php",
                  [], [], $args));
}
<<__EntryPoint>> function main(): void {
runTest("BadAuthDigestTestController");
runTest("GoodAuthDigestTestController");
}
