<?hh

function expect_mixed(mixed $_):void { }

interface I { }

abstract class VC { }

interface IA<T as I> { }

class B<TVC as VC> implements I {
  public function get():TVC {
    throw new Exception("A");
  }
}

function expect_vc(VC $_):void { }

interface IB<-TVC as VC, T as B<TVC>> extends IA<T> {
  public function get_t(): T;
}

class C<T as I> {
  public function testit(T $x, IA<T> $m):void {
    $m as IB<_,_>;
    // This used to be rejected under everything_sdt=true
    expect_mixed($m);
    $x = $m->get_t();
    $y = $x->get();
    // This used to be rejected ALSO under everything_sdt=false
    // It should be accepted
    expect_vc($y);
  }
}
