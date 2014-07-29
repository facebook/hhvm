<?hh

class C1 {}
class C2 extends C1 {}
class C3 extends C2 {}

// being used to test covariance
class PrivacyPolicyBase<Tv> {}

interface I1 {
  require extends PrivacyPolicyBase<C1>;
}
interface I2 extends I1 {
  require extends PrivacyPolicyBase<C2>;
}
interface I3 extends I2 {
  require extends PrivacyPolicyBase<C3>;
}

class Test1 extends PrivacyPolicyBase<C2> implements
  I1,
  I2 {
}

class Test2 extends PrivacyPolicyBase<C3> implements
  I1,
  I2,
  I3 {
}
