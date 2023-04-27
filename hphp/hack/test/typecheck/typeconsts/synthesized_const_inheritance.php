<?hh

abstract class BaseClass {
  const type T = int;
  const int X = 3;
}

trait TTrait {
  require extends BaseClass;
}

class ChildOne extends BaseClass {
  use TTrait;
}

class ChildTwo extends ChildOne {
  const type T = string;
  const string X = "hello";
}
