<?hh
abstract class :base {
  protected static function __xhpAttributeDeclaration() {
    return array();
  }
  public static function xhpAttributeDeclaration() {
    return static::__xhpAttributeDeclaration();
  }
}
class :node1 extends :base {
  attribute array<float> foo;
  attribute array<int, string> bar;
}
function main() {
  var_dump(:node1::xhpAttributeDeclaration()['foo'][0]);
  var_dump(:node1::xhpAttributeDeclaration()['bar'][0]);
}
main();
