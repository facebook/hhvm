<?hh

class Foo {

  <<__AutocompleteSortText("!getFoo")>>
  public function getFoo() : string {
    return 'foo';
  }

  <<__AutocompleteSortText(5)>>
  public function getBar() : string {
    return 'bar';
  }

}
