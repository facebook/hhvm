<?hh // strict

// https://rosettacode.org/wiki/Balanced_brackets#PHP

function isbalanced(string $s): bool {
  $bal = 0;
  for ($i = 0; $i < strlen($s); ++$i) {
    $ch = substr($s, $i, 1);
    if ($ch == '[') {
      $bal++;
    } else if ($ch == ']') {
      $bal--;
    }
    if ($bal < 0)
      return false;
  }
  return ($bal == 0);
}
