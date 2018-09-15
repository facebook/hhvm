<?hh
function foo() {
  if ($a)
    if ($b)
      switch ($c) {
        case 123: fallthrough; // fallthrough parsed but not yet supported
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
