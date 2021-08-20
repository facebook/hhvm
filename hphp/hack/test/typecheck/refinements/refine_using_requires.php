<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class BaseGenericClass<T> {}

interface InterfaceWithRequires<T>{
  require extends BaseGenericClass<T>;
  public function f(): BaseGenericClass<T>;
}

class SubClassAA extends BaseGenericClass<int> implements InterfaceWithRequires<int>{
  public function f(): BaseGenericClass<int>{
    return $this;
  }
}

function type_refinment_test<T>(BaseGenericClass<T> $b): BaseGenericClass<T> {
  if($b is InterfaceWithRequires<_>){
    $x = $b->f();
    return $x;
  }
  return $b;
}
