<?hh

class A {
  public function f(): shape(
    // hello
    'x' => int,
  ) {}
}

class C {
  public function g(): shape(
    // bye
    'y' => int,
  ) {}
}
class D extends C {}

class F {
  public function j(): Awaitable<?shape(
    // cat
    'z' => int,
  )> {}
}
<<__EntryPoint>> function main(): void {
$program = file_get_contents(__FILE__);
$json = HH\ffp_parse_string($program);
$a = HH\ExperimentalParserUtils\find_class_body($json, "A");
invariant($a !== null, "Failed to find class in file");
$shape = HH\ExperimentalParserUtils\find_class_method_shape_return_type($a, "f");
var_dump(HH\ExperimentalParserUtils\extract_shape_comments($shape));


$rd = new ReflectionClass("D");
$description = null;
while ($rd !== false) {
  $d = HH\ExperimentalParserUtils\find_class_body($json, $rd->getName());
  $shape = HH\ExperimentalParserUtils\find_class_method_shape_return_type($d, "g");
  if ($shape === null) {
    $rd = $rd->getParentClass();
  } else {
    $description = HH\ExperimentalParserUtils\extract_shape_comments($shape);
    break;
  }
}
invariant($description !== null, "");
var_dump($description);

$a = HH\ExperimentalParserUtils\find_class_body($json, "F");
$shape = HH\ExperimentalParserUtils\find_class_method_shape_return_type($a, "j");
var_dump(HH\ExperimentalParserUtils\extract_shape_comments($shape));
}
