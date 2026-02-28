<?hh

function test() :mixed{
  var_dump(is_subclass_of('C', 'D'));
  $cls_methods = get_class_methods('C');
  var_dump($cls_methods === null ? null : vec($cls_methods));
  var_dump(method_exists('C', 'foo'));
  include '1377.inc';
  var_dump(is_subclass_of('C', 'D'));
  var_dump(is_subclass_of('C', 'C'));
}

<<__EntryPoint>>
function main_1377() :mixed{
test();
var_dump(class_exists('C'));
}
