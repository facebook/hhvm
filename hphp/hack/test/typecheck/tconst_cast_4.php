<?hh

interface IT1<+Thint> { }
interface IT2<+Thint> { }

interface IBase
{
  abstract const type TID;

  public function getID(): this::TID;
}

interface I0 extends IBase
{ }

interface I1 extends IBase {

  abstract const type TID as IT1<this>;
}
interface I2 extends IBase
{
  abstract const type TID as IT2<this>;
}

class D implements IT1<C>, IT2<C> { }
// Just to prove that we can implement this
final class C implements I1, I2 {
  const type TID = D;
  public function getID(): this::TID {
    return new D();
  }
}

function foo(IT1<I1> $_, IT2<I2> $_):void { }

function top(I1 $x): void {
  $x as I2;
  hh_show($x);
  // So, we call (I1 & I2)->getID()
  // I1->getID() with this=(I1 & I2) produces IT1<(I1 & I2)>
  // I2->getID() with this=(I1 & I2) produces IT2<(I1 & I2)>
  // So result should be IT1<(I1 & I2)> & IT2<(I1 & I2)>
  $y = $x->getID();
  // hh_show_env();
  // hh_show($y);
  foo($y, $y);
}
