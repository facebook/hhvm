<?hh

function autoload_miss($str1, $str2) :mixed{
  echo "Failure handler called: $str1 $str2\n";
}

<<__EntryPoint>>
function entrypoint_type_annotation_autoloader(): void {
  var_dump(type_structure(C::class, 'T'));
  var_dump(type_structure(BuiltinClass::class));
  var_dump(type_structure(LoadedClass::class));
  var_dump(type_structure(AutoloadClass1::class));
  var_dump(type_structure(AutoloadClass2::class));
  var_dump(type_structure(AutoloadType1::class));
  var_dump(type_structure(AutoloadType2::class));

  // ReflectionTypeAlias should autoload the alias it references
  new ReflectionTypeAlias(TestReflectionTypeAlias::class);

  // type_structure() should autoload the alias it references
  var_dump(type_structure(TestTypeStructureTypeAlias::class));
}
