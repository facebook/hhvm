<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface JA { }
interface JB { }
interface JC { }
interface JD { }
class CC implements JC { }

// Has both type and method get
interface IA {
  abstract const type TP as JA;
  public function get(): this::TP;
  public function metha():void;
}
// Just method
interface IB {
  public function get(): JB;
  public function methb():void;
}
// Just type
interface IC {
  abstract const type TP as JC;
  public function methc():void;
  public function getc():this::TP;
}
// Type, but exact
interface ID {
  const type TP = CC;
  public function methd():void;
  public function get():this::TP;
}
interface IEmpty { }

// This is fine, because even though neither IB nor IEmpty have the
// type constant, IA does
function test1<T as (IA & (IB | IEmpty)),TP>(T $x):TP where TP=T::TP {
  return $x->get();
}
// This is not ok
function test2<T as (IB | IEmpty),TP>(T $x):TP where TP=T::TP {
  $y = $x->get();
  //hh_show($y);
  return $y;
}
// Should be possible to return JA & JC
// But right now, we pick the last conjunct
function test3<T as (IA & IC)>(T $x):JC {
  return $x->get();
}
// Ditto
function test4<T as IA as IC>(T $x):JC {
  return $x->get();
}

function testcomplex2<T as (IB & (IA | IC))>(T $x):JB {
  // Expect the type here to be JB & TP & (JA | JC)
  $y = $x->get();
  //hh_show($y);
  return $y;
}


function testcomplex3<T as (IB & IA)>(T $x):(JB & JA) {
  $y = $x->get();
  return $y;
}
