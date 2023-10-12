<?hh

class ConcreteRight {
  const int i1 = 4;
}

class ConcreteWrong {
  const int i1;
}

abstract class AbstractRight {
  abstract const int i1;
}

abstract class AbstractWrong {
  abstract const int i1 = 3;
}

abstract class MixedRight {
  const int i1 = 5;
}

abstract class MixedWrong {
  const int i1;
}
