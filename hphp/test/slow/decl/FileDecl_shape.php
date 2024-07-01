<?hh

<<__EntryPoint>>
function main(): void {
  // Get the autoloader to initialize
  HH\autoload_is_native();

  $text = '<?hh
  type myType = shape("key1" => dict<string, int>, "key2" => string, FAKE_CLASS::FAKE_CONST => string);
  type testType = shape(GaryTestClass::TEST_STRING => int);

    class FooBar {
      const type fooType = shape("k1" => int, BAD_CLS::BAD_CONST => string);
      const type clsTest = shape(GaryTestClass::TEST_STRING => int);
    }
  ';

    $instance = HH\FileDecls::parseText($text);

    var_dump($instance->getShapeKeys('myType'));
    var_dump($instance->getTypeconst("FooBar", "fooType"));
    var_dump($instance->getShapeKeys('testType'));
    var_dump($instance->getTypeconst('FooBar', 'clsTest'));
    var_dump(HH\FileDecls::parseTypeExpression('shape(GaryTestClass::TEST_STRING=>int)'));
}
