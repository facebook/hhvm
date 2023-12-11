<?hh
<<__EntryPoint>> function main(): void {
$pattern =
"/\s([\w_\.\/]+)(?:=([\'\"]?(?:[\w\d\s\?=\(\)\.,'_#\/\\:;&-]|(?:\\\\\"|\\\')?)+[\'\"]?))?/";
$context = "<simpletag an_attribute=\"simpleValueInside\">";

$match = vec[];

  if ($result = preg_match_all_with_matches($pattern, $context, inout $match)) {

var_dump($result);
var_dump($match);
}
}
