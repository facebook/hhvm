<?hh

/* HH_FIXME[4110] */
function my_compact<T>(Vector<?T> $vector): Vector<T> {

}

function test(bool $b): ?string {

  $nullable = Vector { 42, null };
  $non_nullable = my_compact($nullable);

  if ($b) {
    $res = $nullable;
  } else {
    $res = $non_nullable;
  }

  return $res[0];
}
