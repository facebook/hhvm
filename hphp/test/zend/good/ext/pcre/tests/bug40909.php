<?hh
<<__EntryPoint>> function main(): void {
$pattern =
"/\s([\w_\.\/]+)(?:=([\'\"]?(?:[\w\d\s\?=\(\)\.,'_#\/\\:;&-]|(?:\\\\\"|\\\')?)+[\'\"]?))?/";
$context = "<simpletag an_attribute=\"simpleValueInside\">";

$match = vec[];

  $result = preg_match_all_with_matches($pattern, $context, inout $match);
  if ($result) {

var_dump($result);
var_dump($match);
}
}
