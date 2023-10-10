<?hh

namespace Animal;

<<Dog(3)>>
function f(<<Hippo(7)>> $p) :mixed{}

<<Cat(4)>>
type Bear = shape(...);

<<Dolphin(5)>>
class Lion {
  <<Baboon(6)>>
  public int $p;
}

<<Eagle(7)>>
class Zebra extends Lion {}
<<__EntryPoint>> function main(): void {
$rf = new \ReflectionFunction("Animal\\f");
\var_dump($rf->getAttributes());
\var_dump($rf->getAttribute("Dog"));
\var_dump($rf->hasAttribute("Dog"));
$rp = $rf->getParameters()[0];
\var_dump($rp->getAttributes());
\var_dump($rp->getAttribute("Hippo"));
\var_dump($rp->hasAttribute("Hippo"));
$rt = new \ReflectionTypeAlias("Animal\\Bear");
\var_dump($rt->getAttributes());
\var_dump($rt->getAttribute("Cat"));
\var_dump($rt->hasAttribute("Cat"));
$rc = new \ReflectionClass("Animal\\Lion");
\var_dump($rc->getAttributes());
\var_dump($rc->getAttribute("Dolphin"));
\var_dump($rc->hasAttribute("Dolphin"));
$rp = $rc->getProperties()[0];
\var_dump($rp->getAttributes());
\var_dump($rp->getAttribute("Baboon"));
\var_dump($rp->hasAttribute("Baboon"));
$rcc = new \ReflectionClass("Animal\\Zebra");
}
