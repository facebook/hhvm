<?hh

enum Animal: string as string {
  // Dog
  DOG = 'dog';

  // Cat
  CAT = 'cat';
}
<<__EntryPoint>> function main(): void {
$program = file_get_contents(__FILE__);
$json = HH\ffp_parse_string($program);
$e = HH\ExperimentalParserUtils\find_enum_body($json, "Animal");
$description = HH\ExperimentalParserUtils\extract_enum_comments($e);
var_dump($description);
}
