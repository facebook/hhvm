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

function f() {
  if (true) {
    return <span />;
  } else {
    return <div />;
  }
}

async function g() {
  if (true) {
    return <span />;
  } else {
    return <div />;
  }
}

function h() {
  if (true) {
    return <div />;
  } else {
    return null;
  }
}

function i() {
  if (true) {
    return <div />;
  } else if (true) {
    return <span />;
  } else {
    return null;
  }
}
