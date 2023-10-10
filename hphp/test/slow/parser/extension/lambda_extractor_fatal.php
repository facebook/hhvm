<?hh
<<__EntryPoint>> function main(): void {
$a = async ($a) ==> 4; $b = new ReflectionFunction(async ($a) ==> 5);
$program = file_get_contents($b->getFileName());
$json = HH\ffp_parse_string($program);
$allfuns = HH\ExperimentalParserUtils\find_all_functions($json);
}
