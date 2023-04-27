<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH {
  <<__NoAutoDynamic>>
  function dynamic_fun(string $func_name)[]: dynamic;
  function dynamic_class_meth(string $cls_name, string $meth_name)[]: dynamic;
  function dynamic_meth_caller(string $cls_name, string $meth_name)[]: dynamic;
}
