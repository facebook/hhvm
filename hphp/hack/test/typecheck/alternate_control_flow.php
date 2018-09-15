<?hh // strict

function f(int $a): void {
  if ($a == 5):
    echo "a equals 5";
    echo "...";
  elseif ($a == 6):
    echo "lol";
  endif;
}
