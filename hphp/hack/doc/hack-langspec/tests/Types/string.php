<?hh // strict

namespace NS_string;

class C {
  const string GREETING = "Hello";
  private string $prop = 'Now';

  public function setProp(string $val): void {
    $this->prop = $val;
  }

  public function getProp(): string {
    return $this->prop;
  }
}
