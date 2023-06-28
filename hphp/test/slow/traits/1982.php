<?hh
trait THello1 {
  public $hello = "Hello";
}
trait THello2 {
  private $world = "World!\n";
}
class TraitsTest {
  use THello1;
  use THello2;
  function test() :mixed{
    echo $this->hello . ' ' . $this->world;
  }
}
<<__EntryPoint>>
function main_entry(): void {

  error_reporting(E_ALL);
  var_dump(property_exists('TraitsTest', 'hello'));
  var_dump(property_exists('TraitsTest', 'world'));
  $t = new TraitsTest;
  $t->test();
}
