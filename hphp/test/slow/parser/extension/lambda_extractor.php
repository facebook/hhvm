<?hh

class C {
  public async function m() {}

  <<SomeAttribute(3)>>
  public function has_attr() {
    return "hello world";
  }
}
<<__EntryPoint>> function main(): void {
$rl = new ReflectionFunction(async ($a) ==> 4);
$ranon = new ReflectionFunction(function ($b) { return 4; });

$rm = new ReflectionMethod("C", "m");
$ra = new ReflectionMethod("C", "has_attr");
$file = $rm->getFileName();
var_dump($rm->isAsync());
$program = file_get_contents($file);
$json = HH\ffp_parse_string($program);
$allfuns = HH\ExperimentalParserUtils\find_all_functions($json);
invariant(array_key_exists($rm->getStartLine(), $allfuns), "Method m missing");
invariant(array_key_exists($ra->getStartLine(), $allfuns), "Method has_attr missing");
invariant(array_key_exists($rl->getStartLine(), $allfuns), "Lambda missing");
invariant(array_key_exists($ranon->getStartLine(), $allfuns), "Anonymous function missing");
var_dump(HH\ExperimentalParserUtils\body_bounds($allfuns[$ra->getStartLine()]));
var_dump(HH\ExperimentalParserUtils\body_bounds($allfuns[$rl->getStartLine()]));
var_dump(HH\ExperimentalParserUtils\body_bounds($allfuns[$ranon->getStartLine()]));
}
