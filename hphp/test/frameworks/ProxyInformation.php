<?hh
// Put the proxy information in its own "struct-like" class for easy access
// in case folks outside this proxy wall need to change them.
class ProxyInformation {
  const FACEBOOK_PROXY = 'http://fwdproxy:8080';
  // Local network broadcast address; it's impossible to open a TCP connection
  // to this.
  const UNROUTABLE_PROXY = 'http://0.0.0.0:0';

  public static ?Map<string, string> $proxies = null;

  // Determine through a poor man's method whether a proxy will be required to
  // get out to the internet. If we get headers back, then a proxy is not
  // required. If we get false back, then a header is required.
  public static function is_proxy_required(
    string $test_url = 'http://www.google.com'
  ): bool {
    $proxy = null;
    if (Options::$local_source_only) {
      // Intentionally kill internet access
      $proxy = self::UNROUTABLE_PROXY;
    } else if (
      strpos(gethostname(), "facebook.com") || !(get_headers($test_url))
    ) {
      $proxy = self::FACEBOOK_PROXY;
    }

    if ($proxy !== null) {
      self::$proxies = Map {
        "HOME" => getenv("HOME"),
        "http_proxy" => $proxy,
        "https_proxy" => $proxy,
        "HTTPS_PROXY" => $proxy,
        "HTTP_PROXY" => $proxy,
        "HTTP_PROXY_REQUEST_FULLURI" => "true",
        "no_proxy" => "facebook.com,fbcdn.net,localhost,127.0.0.1",
        "NO_PROXY" => "facebook.com,fbcdn.net,localhost,127.0.0.1",
      };
      return true;
    } else {
      self::$proxies = Map { "HOME" => getenv("HOME") };
      return false;
    }
  }
}
