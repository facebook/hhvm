<?hh
function foo() {
  if ($a)
    if ($b)
      switch ($c) {
        case 123: break;
        default: break;
      }
    else
      return $d;
  elseif($e)
    do {
      while($f)
        throw $g;
      continue;
    } while ($h);
}
