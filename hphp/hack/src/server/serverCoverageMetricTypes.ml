(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Coverage_level

(**
 * The type result is an optional trie.
 * The trie leaves are maps from strings of filenames to level_stats for those
 * files.
 * The trie nodes are leaves paired with a map from strings of directory names
 * to another trie.
 *
 *)
type result = level_stats SMap.t trie option
