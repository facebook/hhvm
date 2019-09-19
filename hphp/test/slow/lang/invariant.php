<?hh

class A {}
<<__EntryPoint>> function main(): void {
invariant(true, "yup");
invariant(new A is A, "yup");

invariant(false, "nope");
}
