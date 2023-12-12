<?hh

<<__EntryPoint>>
function main_entry(): void {
  $haystack = "Auf der Stra\xc3\x9fe nach Paris habe ich mit dem Fahrer gesprochen";
  var_dump(
      grapheme_stripos($haystack, 'pariS '),
      grapheme_stristr($haystack, 'paRis '),
      grapheme_substr($haystack, grapheme_stripos($haystack, 'Paris'))
  );
}
