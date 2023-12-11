<?hh


<<__EntryPoint>>
function main_htmlspecialchars_utf8() :mixed{
$inputs = vec[
  "Foo\xc0barbaz",
  "\xc2\"",
  "\xc2\xa0",
  "\xa0",
  "Fooÿñ",
];

foreach ($inputs as $s) {
  $res1 = htmlspecialchars($s, ENT_QUOTES, 'UTF-8');
  echo "'$s' => '$res1'\n";
}
}
