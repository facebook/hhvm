<?hh

namespace {
  const string MYCONST = 'hi test';
}

namespace N {
  const int MYCONST = 1;

  function f(): string {
    return MYCONST;
  }
}
