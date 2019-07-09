<?hh

invariant(true, "yup");

class A {}
invariant(new A is A, "yup");

invariant(false, "nope");
