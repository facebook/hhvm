<?hh // decl    /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
 
// flags for password_hash
const int PASSWORD_DEFAULT = 1;
const int PASSWORD_BCRYPT = 1;

function password_hash(string $Password, int $Hash):string;
function password_verify(string $Password, string $Hash):bool;
