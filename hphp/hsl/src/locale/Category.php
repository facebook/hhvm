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

use namespace HH\Lib\_Private\_Locale;

enum Category: int {
  LC_ALL = _Locale\LC_ALL;
  LC_COLLATE = _Locale\LC_COLLATE;
  LC_CTYPE = _Locale\LC_CTYPE;
  LC_MONETARY = _Locale\LC_MONETARY;
  LC_NUMERIC = _Locale\LC_NUMERIC;
  LC_TIME = _Locale\LC_TIME;
  LC_MESSAGES = _Locale\LC_MESSAGES;
};
