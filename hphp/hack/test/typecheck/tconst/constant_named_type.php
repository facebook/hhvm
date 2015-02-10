<?hh

interface I {
  abstract const type;
}

class C implements I {
  const type = 0;
}

const type = 1;
