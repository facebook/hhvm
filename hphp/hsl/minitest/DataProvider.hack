namespace HH\__Private\MiniTest;

class DataProvider implements \HH\MethodAttribute {
  public function __construct(
    public string $provider,
  ) {
  }
}
