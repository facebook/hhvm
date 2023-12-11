<?hh <<__EntryPoint>> function main(): void {
$arr = vec[
  "This is string one.",
  "This is string two.",
  vec[
      "This is another string.",
      "This is a last string."],
  "This is a last string."];

echo serialize(str_replace("string", "strung", $arr)) . "\n";
echo serialize(str_replace("string", "strung", $arr)) . "\n";
echo serialize(str_replace(" ", "", $arr)) . "\n";
echo serialize(str_replace(" ", "", $arr)) . "\n";
}
