<?hh

class Foo {

  <<__AutocompleteSortText("zzz")>>
  public static function getBaz() : string {
    return 'baz';
  }

  <<__AutocompleteSortText("yyy")>>
  public static function getBoo() : string {
    return 'boo';
  }


  public function aaaac() : string {
    return 'bar';
  }

  <<__AutocompleteSortText("aaaa")>>
  public function getFoo() : string {
    return 'foo';
  }

  <<__AutocompleteSortText("aaaab")>>
  public function getBar() : string {
    return 'bar';
  }

  public function test(): void {
    $this->gAUTO332
  }

}
