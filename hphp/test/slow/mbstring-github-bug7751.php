<?hh


<<__EntryPoint>>
function main_mbstring_github_bug7751() :mixed{
$output = null;
@mb_parse_str("preds=status|=|3;id|>|1000&limit=1", inout $output);
var_dump($output);
}
