<?hh

function mapLookup($map, $key, $default) :mixed{
  return $map->containsKey($key) ? $map[$key] : $default;
}

final class HTTPHeaders {
  private $headers;

  public function __construct() {
    $this->headers = Map {
      'foo' => 'bar',
      'baz' => 'quux',
    }
;
  }

  public function getAllHeaders(): Map<string,string> {
    return $this->headers;
  }

  public function getHeader(string $name, ?string $default = null): ?string {
    return mapLookup($this->getAllHeaders(), strtolower($name), $default);
  }
}

function main() :mixed{
  $h = new HTTPHeaders();
  $val = $h->getHeader('blah', 'wat');
  echo $val;
  echo "\n";
}

<<__EntryPoint>>
function main_headers() :mixed{
main();
}
