<?hh
<<file: __EnableUnstableFeatures('open_tuples')>>

class C {
  const type TV = (int, optional string, optional bool);
  const type TW = (int, nonnull...);
}

function show($x) :mixed{
  print(json_encode($x, JSON_FB_FORCE_HACK_ARRAYS)."\n");
}

<<__EntryPoint>>
function main() :mixed{
  print("type_structure:\n");
  show(type_structure(C::class, 'TV'));
  show(type_structure(C::class, 'TW'));

  $cls = new ReflectionClass('C');
  print("\nReflection: TypeConstants:\n");
  foreach ($cls->getTypeConstants() as $cns) {
    print($cns->getName().': '.$cns->getAssignedTypeText()." =>  ");
    show($cns->getTypeStructure());
  }
}
