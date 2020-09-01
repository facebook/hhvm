<?hh
type Talias = Bogus;
class D {
  const type TD = Bogus;
}
class C {
  const type TC = Bogus;
  const type TC2 = Talias;
  const type TC3 = D;
  public function inst1(this::TC::TT $x):void { }
  public function inst2(this::TC2::TT $x):void { }
  public function inst3(this::TC::TT::TT $x):void { }
  public function inst4(this::TC3::TD::TT $x):void { }
}

function direct(Bogus::TT $x):void { }
function top0(Talias::TT $x):void { }
function top1(C::TC::TT $x):void { }
function top2(C::TC2::TT $x):void { }
function top3(C::TC::TT::TT $x):void { }
function top4(C::TC3::TD::TT $x):void { }
