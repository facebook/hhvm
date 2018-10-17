<?hh

class C {
  const type T = shape(
    // hello
    'a' => int,
  );
}

class D extends C {

}

$rd = new ReflectionClass("D");
$json = HH\ffp_parse_file($rd->getFileName());
$description = null;
while ($rd !== false) {
  $d = HH\ExperimentalParserUtils\find_class_body($json, $rd->getName());
  $shape = HH\ExperimentalParserUtils\find_class_shape_type_constant($d, "T");
  if ($shape === null) {
    $rd = $rd->getParentClass();
  } else {
    $description = HH\ExperimentalParserUtils\extract_shape_comments($shape);
    break;
  }
}
invariant($description !== null);
var_dump($description);
