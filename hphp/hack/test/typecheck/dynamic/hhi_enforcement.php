<?hh

function f(dynamic $d) : void {

  $nf = new NumberFormatter("", 1);
  $nf->getSymbol($d); // error, non-hsl .hhi type are unenforced

  HH\Lib\_Private\Native\pseudorandom_seed($d); // ok, in hsl

}
