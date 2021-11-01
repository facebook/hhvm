<?hh

interface I {
  abstract const int type;
}

class C implements I {
  const int type = 0;
}

const int type = 1;
