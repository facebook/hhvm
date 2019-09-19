<?hh
class C {
        public $prop = 'C::prop.orig';
}
class MyArrayObject extends ArrayObject {
}
function testAccess($c, $ao) {
        foreach ($ao as $key=>$value) {
        }
        unset($ao['prop']);
        var_dump($c->prop, $ao['prop']);
}
<<__EntryPoint>>
function main_entry(): void {
  $c = new C;
  $ao = new MyArrayObject($c);
  testAccess($c, $ao);
}
