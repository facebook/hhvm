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

class SingleConstr <T> where T = int {}

class ListConstrs <T> where T = int T = string {}

class MultiConstrs <T, Tu> where T = int, Tu = string Tu = int {}

class AsConstr <T> where T as int {}

class SuperConstr <T> where T super int {}
