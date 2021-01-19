<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I {
  abstract const type TP as N;
  public function get():this;
  public function getext():this::TP;
  public function imeth():void;
}
interface II {
  abstract const type TP as M;
}
interface J {
  public function jmeth():void;
}
interface K {
  public function get():M;
}
interface M {
  public function mmeth():void;
}
interface N {
  public function nmeth():void;
}

class D implements N {
  public function nmeth():void { echo "D::nmeth"; }
}
class E extends D implements M {
  public function mmeth():void { echo "E::mmeth"; }
}
class C implements I, J {
  const type TP = D;
  public function get():this {
    return $this;
  }
  public function getext():this::TP {
    return new D();
  }
  public function imeth():void {
    echo "C::imeth";
  }
  public function jmeth():void {
    echo "C::jmeth";
  }
}
class CC implements I, II {
  const type TP = E;
  public function get():this {
    return $this;
  }
  public function getext():this::TP {
    return new E();
  }
  public function imeth():void {
    echo "CC::imeth";
  }
}

function get1((I & J) $x, ?(I & J) $xx):void {
  $y = $x->get();
//  hh_show($y);
  $y->imeth();
  $y->jmeth();

  $yy = $xx?->get();
//  hh_show($yy);
  $yy?->imeth();
  $yy?->jmeth();

  $a = $x->getext();
//  hh_show($a);
  $a->nmeth();
}
function get3((I & II) $x, ?(I & II) $xx):void {
  $y = $x->get();
  //hh_show($y);
  $y->imeth();

  $yy = $xx?->get();
  //hh_show($yy);
  $yy?->imeth();

  $a = $x->getext();
  //hh_show($a);
  $a->mmeth();
  $a->nmeth();
}
function get2((I & K) $x, ?(I & K) $xx):void {
  $y = $x->get();
  //hh_show($y);
  $y->imeth();
  $y->mmeth();

  $yy = $xx?->get();
  //hh_show($yy);
  $yy?->imeth();
  $yy?->mmeth();
}

interface JA { }
interface JB { }
interface JC { }
// Has both type and method
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
  public function get():this::TP;
}

function testcomplex((IA & (IB | IC)) $x):JA {
  return $x->get();
}

function testcomplex2((IB & (IA | IC)) $x):(JB & (JA | JC)) {
  $y = $x->get();
  return $y;
}
