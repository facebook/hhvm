<?hh
var_dump($_SERVER['Authorization']);
var_dump(apache_request_headers()['Authorization']);
var_dump($_ENV['HTTP_PROXY'] ?? 'NOT SET');
var_dump(getenv('HTTP_PROXY'));
