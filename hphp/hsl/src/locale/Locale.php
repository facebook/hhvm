<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Locale;

/** An object representing a locale and related settings.
 *
 * This also encapsulates the various `LC_*` settings, so, for example,
 * `LC_CTYPE` can indicate UTF-8, and `LC_COLLATE` and `LC_NUMERIC` can
 * be set to differing locations (e.g. `en_US` or and `fr_FR`).
 */
type Locale = \HH\Lib\_Private\_Locale\Locale;
