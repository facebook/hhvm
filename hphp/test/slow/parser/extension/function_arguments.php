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
<<__EntryPoint>> function main(): void {
$rm = new ReflectionMethod("C", "f");
$program = file_get_contents($rm->getFileName());
$json = HH\ffp_parse_string($program);
$ma = HH\ExperimentalParserUtils\find_method_parameters($json,
  $rm->getName(), $rm->getStartLine());
var_dump(HH\ExperimentalParserUtils\extract_parameter_comments($ma));
}
