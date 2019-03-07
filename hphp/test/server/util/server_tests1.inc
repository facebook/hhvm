<?hh

function request($host, $port, $path, $timeout = 1200) {
  return http_request($host, $port, $path, $timeout);
}
