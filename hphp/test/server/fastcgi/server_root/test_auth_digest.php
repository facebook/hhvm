<?hh

<<__EntryPoint>>
function test_auth_digest_entrypoint() :mixed{
    $realm = 'Restricted area';

    //user => password
    $users = dict['admin' => 'mypass'];

    if (!($_SERVER['PHP_AUTH_DIGEST'] ?? false)) {
        header('HTTP/1 . 1 401 Unauthorized');
        header('WWW-Authenticate: Digest realm="' . $realm .
            '",qop="auth",nonce="' . uniqid() . '",opaque="' .
            md5($realm) . '"');

        exit('Text to send if user hits Cancel button');
    }


    // analyze the PHP_AUTH_DIGEST variable
    if (!($data = http_digest_parse($_SERVER['PHP_AUTH_DIGEST'])) ||
        !isset($users[$data['username']]))
        exit('Wrong Credentials!');


    // generate the valid response
    $A1 = md5($data['username'] . ':' . $realm . ':' . $users[$data['username']]);
    $A2 = md5($_SERVER['REQUEST_METHOD'] . ':' . $data['uri']);
    $valid_response = md5($A1 . ':' . $data['nonce'] . ':' . $data['nc'] . ':' .
                        $data['cnonce'] . ':' . $data['qop'] . ':' . $A2);

    if ($data['response'] != $valid_response)
        exit('Wrong Credentials!');

    // ok, valid username & password
    echo 'You are logged in as: ' . $data['username'];
}


// function to parse the http auth header
function http_digest_parse($txt)
:mixed{
    // protect against missing data
    $needed_parts = dict['nonce'=>1, 'nc'=>1, 'cnonce'=>1, 'qop'=>1,
                          'username'=>1, 'uri'=>1, 'response'=>1];
    $data = dict[];
    $keys = implode('|', array_keys($needed_parts));

    $matches = null;
    preg_match_all_with_matches(
      '@('.$keys.')=(?:([\'"])([^\2]+?)\2|([^\s,]+))@',
      $txt,
      inout $matches,
      PREG_SET_ORDER,
    );

    foreach ($matches as $m) {
        $data[$m[1]] = $m[3] ? $m[3] : $m[4];
        unset($needed_parts[$m[1]]);
    }

    return $needed_parts ? false : $data;
}
