<?hh

<<__SupportDynamicType>>
function first(vec<int> $vi):int {
  return $vi[0];
  //     ^ hover-at-caret
}
