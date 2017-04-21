<?hh

function autoload_miss($str1, $str2) {
  echo "Failure handler called: $str1 $str2\n";
}

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
    'failure' => fun('autoload_miss'),
  },
  __DIR__.'/'
);


class C {
  const type T = this;
}

var_dump(type_structure(C::class, 'T'));

// Test that we do not attempt to autoload a bultin class
type BuiltinClass = Indexish;
type_structure(BuiltinClass::class);

// Test that we do not attempt to autoload an already loaded class
type LoadedClass = C;
type_structure(LoadedClass::class);

// Test that we autoload a class in another file
type AutoloadClass1 = TestAutoloadedClass1;
type_structure(AutoloadClass1::class);

// Test that we don't autoload an already loaded class
type AutoloadClass2 = TestAutoloadedClass2;
type_structure(AutoloadClass2::class);

// The same for aliases
type AutoloadType1 = TestAutoloadedType1;
type_structure(AutoloadType1::class);

type AutoloadType2 = TestAutoloadedType2;
type_structure(AutoloadType2::class);

// ReflectionTypeAlias should autoload the alias it references
new ReflectionTypeAlias(TestReflectionTypeAlias::class);

// type_structure() should autoload the alias it references
type_structure(TestTypeStructureTypeAlias::class);
