<?hh

abstract class BN {
  const type TUniverseRenderer = string;
}

abstract class FBN extends BN {
  const type TUniverseRenderer = string;
}

interface IN {
  require extends BN;
}

interface I extends IN {
  require extends FBN;
}
