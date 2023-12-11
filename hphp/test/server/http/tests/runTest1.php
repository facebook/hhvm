<?hh

<<__EntryPoint>> function main(): void {
  require_once('test_base.inc');
  init();
requestAll(vec[
  "test_status.php",
  "test_get.php?name=Foo",
  "test_get.php?name=Bar",
  "apc_treadmill.php",
  "apc_treadmill.php",
  "apc_treadmill.php",
  "apc_apache_note.php",
  "apc_apache_note.php",
  "apc_apache_note.php",
  // This is expected to pass.
  "test_cookie.php?cookie_name=asdf&cookie_value=foo&cookie_path=BAR",
  // This is expected to fatal due to embedded newline in the cookie name.
  "test_cookie.php?cookie_name=as%0d%0adf&cookie_value=foo&cookie_path=BAR",
  // This will pass even though there is a newline in the value.
  // It is only checked under certain circumstances.
  "test_cookie.php?cookie_name=asdf&cookie_value=f%0d%0aoo&cookie_path=BAR",
  // This is expected to fatal due to embedded newline in the cookie path.
  "test_cookie.php?cookie_name=asdf&cookie_value=foo&cookie_path=B%0d%0aAR",
  // This is expected to fatal due to embedded newline in the cookie domain.
  "test_cookie.php?cookie_name=asdf&cookie_value=foo&cookie_domain=B%0d%0aAR",
  // This is expected to pass.
  "test_cookie.php?cookie_name=asdf&cookie_value=foo=bar",
  // This is expected to pass.
  "test_cookie.php?cookie_name=asdf&cookie_value=foo&cookie_path=B=AR",
  // This is expected to pass.
  "test_header.php?test_string=foo",
  // This is expected to fatal due to embedded newline in the header.
  "test_header.php?test_string=f%0d%0aoo",
  vec[
    "test_duplicate_headers.php", null, dict[
      'xyzzy' => 42, 'XyZZy' => 43, 'XYZZY' => 44, 'xxxxx' => 45
    ]
  ],
  "test_curl_multi_except.php",
  "test_pagelet.php",
  "test_cookie2.php",
  "test_curl_empty_post.php",
]);
}
