<?hh

// Minimal stand-in for the real DataProvider
final class DataProvider implements HH\MethodAttribute {

  public function __construct(string ...$providers)[] {}

}
