<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class :xhp implements XHPChild {}

class :foo extends :xhp {}

function with_xhp(): :xhp {
  return <foo/>;
}
