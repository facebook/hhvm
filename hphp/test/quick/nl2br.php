<?hh
//Tests issue #3815

//This makes reading the output easier for debugging
function escapeNewLine($string) :mixed{
  return str_replace(vec["\n", "\r"], vec['\n', '\r'], $string);
}
<<__EntryPoint>> function main(): void {
$stringList = vec[
  "Test\nString",
  "Test\rString",
  "Test\n\rString",
  "Test\r\nString",
  "Test\n\nString",
  "Test\r\rString",
  "Test String\n",
  "Hello<br />\nmy<br />\r\nfriend<br />\n\r" //case from issue
];
foreach ($stringList as $string) {
  var_dump(escapeNewLine(nl2br($string, true)));
  var_dump(escapeNewLine(nl2br($string, false)));
}
}
