<?hh // strict

$a = async ($a) ==> 4; $b = new ReflectionFunction(async ($a) ==> 5);
$json = HH\ffp_parse_file($b->getFileName());
$allfuns = ReflectionFunctionAbstract::find_all_functions($json);
