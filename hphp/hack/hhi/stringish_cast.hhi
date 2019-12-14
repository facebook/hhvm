<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH;

/**
 * This function converts any type to a string.
 * This conversion may be lossy.
 * When you know that you have a primitive / scalar type,
 * casts using (string) are preferred.
 */
function stringish_cast(mixed $value): string;