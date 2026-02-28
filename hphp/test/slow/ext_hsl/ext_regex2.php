<?hh

<<__EntryPoint>>
function main(): void {
  // Build up a really long pattern made of well-defined names.
  $pat = "";
  for ($i = 0; $i < 300; $i++) {
    $pat .= sprintf("(?"."<a%04x>b%04x)?", $i, $i);
  }

  $matches = dict[];
  $flags = PREG_FB__PRIVATE__HSL_IMPL;
  preg_match_with_matches("/$pat/", "", inout $matches, $flags);
  $last = null;
  foreach ($matches as $k => $v) {
    if ($last is null) {
      $last = $k as int;
    } else {
      echo str_pad(dechex($last), 4, "0", STR_PAD_LEFT), " => $k\n";
      $last = null;
    }
  }
}
