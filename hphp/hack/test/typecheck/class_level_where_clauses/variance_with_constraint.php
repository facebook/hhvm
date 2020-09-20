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

class CovarianceBase<+T> where T as num {}
class ContravarianceBase<-T> where T super int {}
class Base<T> where T as num {}
function Coariance(CovarianceBase<int> $b_int): CovarianceBase<num> {
  return $b_int;
}
function ErrorCoariance(CovarianceBase<num> $b_num): CovarianceBase<int> {
  return $b_num;
}
function Contravariance(ContravarianceBase<num> $b_num)
  : ContravarianceBase<int> {
  return $b_num;
}
function ErrorContravariance(ContravarianceBase<int> $b_int)
  : ContravarianceBase<num> {
  return $b_int;
}
function ErrorUpcast(Base<int> $b_int): Base<num> {
  return $b_int;
}
