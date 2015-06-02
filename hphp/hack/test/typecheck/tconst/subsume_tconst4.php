<?hh // strict

interface P {
  abstract const type T as arraykey;
}

interface C extends P {
  abstract const type T as mixed;
}
