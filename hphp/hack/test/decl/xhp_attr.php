<?hh

class :bar {}

class :foo {
  public ?string $before;
  attribute int a = 0;
  attribute int b;
  attribute int c @required;
  attribute ?int d = null;
  attribute enum {'a', 'b'} e;
  attribute int f @lateinit;
  attribute mixed g;
  attribute :bar;
  public ?string $after;
}
