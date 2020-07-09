<?hh
abstract class :base {
  protected static function __xhpAttributeDeclaration() {
    return darray[];
  }
  public static function xhpAttributeDeclaration() {
    return static::__xhpAttributeDeclaration();
  }
}
class :node1 extends :base {
  attribute varray<float> foo;
  attribute darray<int, string> bar;
}
function main() {
  var_dump(:node1::xhpAttributeDeclaration()['foo'][0]);
  var_dump(:node1::xhpAttributeDeclaration()['bar'][0]);
}

<<__EntryPoint>>
function main_xhp_attr_array_with_typeargs() {
main();
}
