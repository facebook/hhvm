<?hh

enum A : int {}
enum B : int {}
enum C : int {}

<<__SupportDynamicType>>
function f(A $a): void {
  hh_show($a);

  $c1 = (B $b) ==> {
    hh_show($a);
    hh_show($b);

    $c2 = (C $c) ==> {
      hh_show($a);
      hh_show($b);
      hh_show($c);
    };
  };
}
