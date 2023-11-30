<?hh

class Foo {

  <<__AutocompleteSortText("patricia")>>
  public static function getBaz() : string {
    return 'baz';
  }

  <<__AutocompleteSortText("nikhil")>>
  public static function getBrr() : string {
    return 'brr';
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
    $this::gAUTO332
  }

}
