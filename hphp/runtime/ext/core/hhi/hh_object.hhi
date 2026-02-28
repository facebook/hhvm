<?hh
// Copyright (c) 2022, Meta, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// See T119877761: Hack no longer has a definition for `object`, but it's used
// often in systemlib as a type to mean "any class," mapping to `KindOfObject`
// in HHVM incidentally (see: HPHP::get_datatype). `dynamic` as a type is good
// enough, given most functions that take in `object` tend to do dynamic
// operations on them anyways.
//
// Claim both object and HH\object before we autoimport object into the HH ns.
namespace HH {
  type object = dynamic;
}
