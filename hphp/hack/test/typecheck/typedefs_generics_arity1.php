<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
class B<T> {

}

type C = shape(
  'b' => classname<B>,
);
