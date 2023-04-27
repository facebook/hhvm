<?hh

abstract class BadWithoutExperimental {
  abstract const type T super string;
  abstract const type Tdflt super int = num;

  abstract const type Tboth1 as arraykey super string;
  abstract const type Tboth2 super string as arraykey;
}
