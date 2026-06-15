<?hh


class C {
  const type TV = shape('a' => int, ?'b' => string);
  const type TW = shape('a' => int, string...);
  const type TX = shape('a' => int, mixed...);
}

function show($x) :mixed{
  print(json_encode($x, JSON_FB_FORCE_HACK_ARRAYS)."\n");
}

<<__EntryPoint>>
function main() :mixed{
  print("type_structure:\n");
  show(type_structure(C::class, 'TV'));
  show(type_structure(C::class, 'TW'));
  show(type_structure(C::class, 'TX'));

  $cls = new ReflectionClass('C');
  print("\nReflection: TypeConstants:\n");
  foreach ($cls->getTypeConstants() as $cns) {
    print($cns->getName().': '.$cns->getAssignedTypeText()." =>  ");
    show($cns->getTypeStructure());
  }
}
