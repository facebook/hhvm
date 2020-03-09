// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use im_rc::{OrdMap, OrdSet};

use oxidized::local_id::LocalId;

pub type ISet = OrdSet<isize>;

pub type SSet<'a> = OrdSet<&'a str>;

pub type IMap<V> = OrdMap<isize, V>;

pub type SMap<'a, V> = OrdMap<&'a str, V>;

pub type LocalIdMap<'a, V> = OrdMap<&'a LocalId, V>;
