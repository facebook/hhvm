<?hh

abstract class BN {
  const FOO = "one";
}

abstract class FBN extends BN {
  const FOO = "two";
}

interface IN {
  require extends BN;
}

interface I extends IN {
  require extends FBN;
}
