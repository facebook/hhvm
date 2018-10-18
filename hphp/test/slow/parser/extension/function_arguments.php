<?hh

class B {
  public function f(
    // not this one
    int $z,
  ): string {}
}

class C {
  public function f(
    int $a, // Description of a.
    // Description of the
    // parameter b.
    int $b,
    int $c,
  ): string {}
}

$rm = new ReflectionMethod("C", "f");
$json = HH\ffp_parse_file($rm->getFileName());
$ma = HH\ExperimentalParserUtils\find_method_parameters($json,
  $rm->getName(), $rm->getStartLine());
var_dump(HH\ExperimentalParserUtils\extract_parameter_comments($ma));
