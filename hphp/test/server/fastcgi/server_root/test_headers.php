<?hh
<<__EntryPoint>>
function test_https_headers() :mixed{
  var_dump($_SERVER['Authorization']);
  var_dump(\HH\get_headers_secure()['authorization'][0] ?? null);
  var_dump($_ENV['HTTP_PROXY'] ?? 'NOT SET');
  var_dump(getenv('HTTP_PROXY'));
}
