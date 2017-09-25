<?hh

invariant(true, "yup");

class A {}
invariant(new A instanceof A, "yup");

invariant(false, "nope");
