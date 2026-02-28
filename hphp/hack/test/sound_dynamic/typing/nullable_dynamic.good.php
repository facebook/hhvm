<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class C {
  const type TC = shape('a' => int);
}

<<__SupportDynamicType>>
function getShape():~C::TC {
  return shape('a' => 3);
}

<<__SupportDynamicType>>
function getVec():~vec<int> {
  return vec[];
}


<<__SupportDynamicType>>
function shapeget(vec<int> $vi):void {
  $s = null;
  try {
    $s = getShape();
  } finally {
    if ($s is nonnull) { }
  }
  // $s ends up with type ?dynamic in dynamic checking mode
  $i = $s['a'];
}

<<__SupportDynamicType>>
function shapeput(vec<int> $vi):void {
  $s = null;
  try {
    $s = getShape();
  } finally {
    if ($s is nonnull) { }
  }
  // $s ends up with type ?dynamic in dynamic checking mode
  $s['a'] = 1;
}

<<__SupportDynamicType>>
function vecappend(vec<int> $vi):void {
  $s = null;
  try {
    $s = getVec();
  } finally {
    if ($s is nonnull) { }
  }
  // $s ends up with type ?dynamic in dynamic checking mode
  $s[] = 1;
}
