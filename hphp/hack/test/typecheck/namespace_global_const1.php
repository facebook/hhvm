<?hh

namespace {
  const string MYCONST = 'hi test';
}

namespace N {
  function f(): string {
    return MYCONST;
  }
}
