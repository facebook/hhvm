<?hh

function f($a) {
  if ($a) {
  } else
    if ($a) /* hi */
    {
  } else /* hi */ if ($a)
    {
  } elseif ($a) {
  } else {
  }
}
