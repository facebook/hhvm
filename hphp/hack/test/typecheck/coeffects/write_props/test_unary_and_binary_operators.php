<?hh

class Foo {

  public function __construct(
    public int $prop_int,
    public num $prop_num,
    public string $prop_str
   ) {}

}

// Error on each line
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
  $x->prop_int++;
  $x->prop_int--;
  ++$x->prop_int;
  --$x->prop_int;
}

// No errors
function write_props_function(Foo $x)[write_props] : void {
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
  $x->prop_int++;
  $x->prop_int--;
  ++$x->prop_int;
  --$x->prop_int;
}
