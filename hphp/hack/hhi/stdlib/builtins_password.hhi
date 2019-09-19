<?hh    /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// flags for password_hash
const int PASSWORD_DEFAULT = 0;
const int PASSWORD_BCRYPT = 0;

<<__PHPStdLib>>
function password_hash(string $password, int $algo, darray $options = darray[]): ?string;
<<__PHPStdLib>>
function password_verify(string $password, string $hash): bool;
<<__PHPStdLib>>
function password_get_info(string $hash): darray;
<<__PHPStdLib>>
function password_needs_rehash(string $password, int $algo, darray $options = darray[]): bool;
