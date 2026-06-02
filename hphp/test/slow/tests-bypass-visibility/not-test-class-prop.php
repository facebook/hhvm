<?hh

class BestObject {
  <<__TestsBypassVisibility>>
  private int $priv_prop = 1;
}

class WWWTest {}

class NotATest {
  public function test(): void {
    $obj = new BestObject();
    var_dump($obj->priv_prop);
  }
}

<<__EntryPoint>>
function main(): void {
  (new NotATest())->test();
}
