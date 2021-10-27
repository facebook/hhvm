<?hh

class Foo {

  public function __construct(
    public int $prop_int,
    public num $prop_num,
    public string $prop_str
   ) {}

}

function pure_function(Foo $x)[] : void {
  $x->prop_int = 4;
  $x->prop_int += 4;
  $x->prop_int -= 4;
  $x->prop_int *= 4;
  $x->prop_num /= 4;
  $x->prop_num **= 4;
  $x->prop_int %= 4;
  $x->prop_int &= 4;
  $x->prop_int |= 4;
  $x->prop_int <<= 4;
  $x->prop_int >>= 4;
  $x->prop_int ^= 4;
  $x->prop_str ??= 'default';
  $x->prop_str ??= "some text";
  $x->prop_str .= 'more text';
}
