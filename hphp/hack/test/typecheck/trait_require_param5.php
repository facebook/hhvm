<?hh

// Repeated use of Tv bound to something different in each class is meant
// to be a stress-test of tricksy typing_decl instantiation code

abstract class Super<Tk, Tv> {}

abstract class SuperChild<Tv> extends Super<array<mixed>, Tv> {}

trait TReq<Tk, Tv> {
  require extends Super<Tk, Awaitable<Tv>>;
}

class C1 extends SuperChild<Awaitable<bool>> {
  use TReq<array<mixed>, bool>;
}
