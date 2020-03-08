<?hh // strict

namespace NS_require_extends_implements;

class B {
}

class C extends B {
}

/* While multiple class types can be require extends, given the single-
inheritance model, all such class types have to be related via inheritance
for the implementing class tobe able to extend them all. */

interface I {
  const int COUNT1 = 100;
  require extends B;
  const int COUNT2 = 1000;
  require extends C;
  const int COUNT3 = 10000;
}

// As D extends C, which in turn extends B, D satisfies I's requirement

class D extends C implements I {
}

interface J {
}

interface K {
}

trait T {
  const int COUNT1 = 1;
  require extends B;
  const int COUNT2 = 10;
  require implements J;
  const int COUNT3 = 100;
  require extends C;
  const int COUNT4 = 1000;
  require implements K;
  const int COUNT5 = 10000;
}

// As E extends C, which in turn extends B, E satisfies T's requirement
// re base classes

class E extends C implements J, K {
  use T;
}

function main (): void
{
}

/* HH_FIXME[1002] call to main in strict*/
main();
