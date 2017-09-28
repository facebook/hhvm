<?hh
class A {
  public function __construct(
    <<Annotation>>
     $property
  ) {}
}

function f(
    <<A1>> $p1,
    <<A2>> $p2
) {

}

function test($caption, $parameters, $i, $attribute) {
    $p = $parameters[$i]->getAttribute($attribute);
    print $caption . ":" . (isset($p) ? "SET" : "NOTSET") . "\n";
}

function run() {
    $cls = new ReflectionClass("A");
    $ctor = $cls->getConstructor();
    test('1. existing attribute', $ctor->getParameters(), 0, "Annotation");
    test('2. missing attribute', $ctor->getParameters(), 0, "Annotation-missing");

    $func = new ReflectionFunction('f');
    test('3. existing attribute', $func->getParameters(), 0, "A1");
    test('4. missing attribute', $func->getParameters(), 0, "A1-missing");
    test('5. existing attribute', $func->getParameters(), 1, "A2");
    test('6. missing attribute', $func->getParameters(), 1, "A2-missing");
}

run();
