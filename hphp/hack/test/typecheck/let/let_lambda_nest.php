<?hh // experimental

function foo(): void {
  let amount = 10;
  let incr = ($x) ==> {
    let incr1 = ($x) ==> $x + amount;
    let incr2 = ($x) ==> incr1(incr1($x));
    let incr3 = ($x) ==> {
      let t = incr2($x);
      return t + t;
    };
    return incr3(incr2(incr1($x)));
  };
  echo incr(amount);
}
