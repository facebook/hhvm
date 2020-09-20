<?hh
class A {
    const HW = "this is A";
}
class B extends A {
    const BHW = parent::HW . " extended by B";
}
const C = B::BHW;
<<__EntryPoint>> function main(): void {
echo C, "\n";
}
