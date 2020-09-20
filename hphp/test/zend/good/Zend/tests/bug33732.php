<?hh
interface iA {
  const cA = "const of iA\n";
}

class A implements iA {
}

class B extends A implements iA {
}

interface iA2 {
    const cA = "const of iA2\n";
}

interface iB2 extends iA2 {
}

class A2 implements iA2 {
}

class B2 extends A2 implements iA2 {
}
<<__EntryPoint>> function main(): void {
echo iA::cA;
echo A::cA;
echo B::cA;

echo iA2::cA;
echo A2::cA;
echo iB2::cA;
echo B2::cA;
}
