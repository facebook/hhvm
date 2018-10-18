<?hh

enum Animal: string as string {
  // Dog
  DOG = 'dog';

  // Cat
  CAT = 'cat';
}

$json = HH\ffp_parse_file(__FILE__);
$e = HH\ExperimentalParserUtils\find_enum_body($json, "Animal");
$description = HH\ExperimentalParserUtils\extract_enum_comments($e);
var_dump($description);
