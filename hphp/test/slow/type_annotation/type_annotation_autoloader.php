<?hh

function autoload_miss($str1, $str2) {
  echo "Failure handler called: $str1 $str2\n";
}

<<__EntryPoint>>
function entrypoint_type_annotation_autoloader(): void {

  HH\autoload_set_paths(
    Map {
      'class' => Map {
        'testautoloadedclass1' => 'type_annotation_autoloader-1.inc',
        'testautoloadedclass2' => 'type_annotation_autoloader-1.inc',
        'testfoo' => 'type_annotation_autoloader-3.inc',
      },
      'type' => Map {
        'testautoloadedtype1' => 'type_annotation_autoloader-2.inc',
        'testautoloadedtype2' => 'type_annotation_autoloader-2.inc',
        'testreflectiontypealias' => 'type_annotation_autoloader-3.inc',
        'testtypestructuretypealias' => 'type_annotation_autoloader-4.inc',
      },
      'failure' => autoload_miss<>,
    },
    __DIR__.'/'
  );

  require_once(__DIR__.'/type_annotation_autoloader-main.inc');

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
