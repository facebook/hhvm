<?hh
// Put the proxy information in its own "struct-like" class for easy access
// in case folks outside this proxy wall need to change them.
class ProxyInformation {
  const FACEBOOK_PROXY = 'http://fwdproxy:8080';

  public static ?Map<string, string> $proxies = null;

  // Determine through a poor man's method whether a proxy will be required to
  // get out to the internet. If we get headers back, then a proxy is not
  // required. If we get false back, then a header is required.
  public static function is_proxy_required(
                             string $test_url = 'http://www.google.com'): bool {
    if (strpos(gethostname(), "facebook.com") || !(get_headers($test_url))) {
      self::$proxies =
        Map {
          "HOME" => getenv("HOME"),
          "http_proxy" => self::FACEBOOK_PROXY,
          "https_proxy" => self::FACEBOOK_PROXY,
          "HTTPS_PROXY" => self::FACEBOOK_PROXY,
          "HTTP_PROXY" => self::FACEBOOK_PROXY,
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
