<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class MyParent {

  const type MyParentShape = shape(
  'value1' => int,
  'inner' => MyParentShapeFix, // for some reason this is fine?!
  // 'inner' => MyParentShape, // is an recursive error
  ...
);
}

class MyChild {
  const type MyChildShape = shape(
  'value1' => int,
  'value2' => string,
  'inner' => MyChildShapeFix, // for some reason this is fine?!
  // 'inner' => MyChildShape, // is an recursive error
);
}


// sprinkle in a bit of indirection
type MyParentShapeFix = MyParent::MyParentShape;
type MyChildShapeFix = MyChild::MyChildShape;

// Demo how MyParentShape and MyChildShape are broken
  function foo(MyParent::MyParentShape $x): MyChild::MyChildShape {
  // MyChildShape isn't a subtype of MyParentShape (it's the other way around)
  // return $x; // this is an error, (expected)
  return $x['inner']; // this should also be an error but isn't
}
