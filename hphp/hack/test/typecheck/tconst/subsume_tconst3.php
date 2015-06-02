<?hh // strict

class P {
  const type T as arraykey = string;
}

class C extends P {
  const type T = int;
}
