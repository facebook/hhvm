<?hh

<<file:__EnableUnstableFeatures('case_types')>>

class A {}
class B {}

case type C1 = int | string | float | null;

case type C2 = bool | A;
case type C3 = B | C2;

case type C4 = (C2, (C2, C3));

function pp(string $type, string $name) :mixed{
  echo ">>>> " . $type . " <<<<\n\n";
  var_export(type_structure_for_alias($name));
  echo "\n\n";
  echo (new ReflectionTypeAlias($name))->getAssignedTypeText();
  echo "\n\n";
}

<<__EntryPoint>>
function main() :mixed{
  pp("Basic", 'C1');
  pp("Class", 'C2');
  pp("Nested", 'C3');
  pp("Very nested", 'C4');
}
