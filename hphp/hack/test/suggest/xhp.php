<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class :xhp {}
class :div extends :xhp {}
class :span extends :xhp {}

function f(bool $b) {
  if ($b) {
    return <span />;
  } else {
    return <div />;
  }
}

async function g(bool $b) {
  if ($b) {
    return <span />;
  } else {
    return <div />;
  }
}

function h(bool $b) {
  if ($b) {
    return <div />;
  } else {
    return null;
  }
}

function i(bool $b) {
  if ($b) {
    return <div />;
  } else if ($b) {
    return <span />;
  } else {
    return null;
  }
}
