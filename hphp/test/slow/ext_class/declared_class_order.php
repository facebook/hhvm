<?hh
class A {
}


<<__EntryPoint>>
function main_declared_class_order() :mixed{
$classes = get_declared_classes();
var_dump(count($classes) > 1);
var_dump($classes[count($classes) - 1]);
}
