<?hh

<<__SupportDynamicType>>
function first(bool $b, vec<int> $vi): vec<int> {
  if ($b) {
    return $vi;
  } else
    $x = vec[3];
    return $x;
    //     ^ hover-at-caret
  }
}
