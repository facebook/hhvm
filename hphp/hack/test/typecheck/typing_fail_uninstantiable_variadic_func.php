<?hh // strict
/**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

abstract final class CannotBeInstantiatedWithT<T> {}
function errorWithT<T>(CannotBeInstantiatedWithT<T> ...$args): void {}
