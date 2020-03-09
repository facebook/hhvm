// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::InternalType;

use im_rc::OrdSet;

pub type ITySet<'a> = OrdSet<InternalType<'a>>;
