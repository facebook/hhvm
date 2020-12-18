<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface J { }
interface K { }
class C implements J { }
class D { }

// Abstract, with bound
interface IBoundedJ {
  abstract const type TP as J;
  public function get():this::TP;
  public function set(this::TP $_):void;
}
interface IBoundedK {
  abstract const type TP as K;
  public function get():this::TP;
  public function set(this::TP $_):void;
}

// Abstract, without bound
interface IUnbounded {
  abstract const type TP;
  public function get():this::TP;
  public function set(this::TP $_):void;
}

interface IMissing {
  public function somemethod():void;
}

interface IConcreteJ {
  const type TP = J;
}
interface IConcreteK {
  const type TP = K;
}

// All should pass type checker
function test1<T as (IMissing & IBoundedJ)>(T $x):J {
  return $x->get();
}

function test2<T as (IBoundedJ & IUnbounded)>(T $x):J {
  return $x->get();
}

function test3<T as (IBoundedJ & IBoundedK)>(T $x):(J & K) {
  return $x->get();
}

function test4<T as (IUnbounded & IConcreteJ)>(T $x):J {
  $x->set(new C());
  return $x->get();
}

// Actually uninstantiable, but no need to reject
function test5<T as (IUnbounded & IConcreteJ & IConcreteK)>(T $x, (J & K) $y):(J & K) {
  $x->set($y);
  return $x->get();
}

// IBounded is redundant
function test6<T as (IUnbounded & IBoundedJ & IConcreteJ)>(T $x, J $y):J {
  $x->set($y);
  return $x->get();
}

function test7<T as (IBoundedJ | IUnbounded)>(T $x):mixed {
  return $x->get();
}

// Currently we don't allow unioning of abstract type constants
// In future, we will support this
/*
function test8<T as IUnbounded as (IBoundedJ | IBoundedK)>(T $x):(J | K) {
  return $x->get();
}

function test9<T as IUnbounded as (IConcreteJ | IBoundedJ)>(T $x):J {
  return $x->get();
}
*/
