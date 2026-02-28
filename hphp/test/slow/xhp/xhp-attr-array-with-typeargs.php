<?hh
abstract class :base {
  protected static function __xhpAttributeDeclaration()[] :mixed{
    return dict[];
  }
  public static function xhpAttributeDeclaration() :mixed{
    return static::__xhpAttributeDeclaration();
  }
}
class :node1 extends :base {
  attribute varray<float> foo;
  attribute darray<int, string> bar;
}
function main() :mixed{
  var_dump(:node1::xhpAttributeDeclaration()['foo'][0]);
  var_dump(:node1::xhpAttributeDeclaration()['bar'][0]);
}

<<__EntryPoint>>
function main_xhp_attr_array_with_typeargs() :mixed{
main();
}
