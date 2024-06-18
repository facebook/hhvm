<?hh
class Bar {}

<<__EntryPoint>>
function main() :mixed{
  var_dump(HH\is_class(Bar::class));
  var_dump(HH\is_class("Bar"));
  var_dump(HH\class_to_classname(Bar::class));
  var_dump(HH\class_to_classname("Bar"));
}
