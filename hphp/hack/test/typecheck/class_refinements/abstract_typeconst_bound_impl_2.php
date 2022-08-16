<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface IUserData {}

abstract class FrameworkA {
  abstract const type TB as FrameworkB with {type TUserData = this::TUserData};
  abstract const type TUserData as IUserData;
}

abstract class FrameworkB {
  abstract const type TUserData as IUserData;
}

class UserDataA implements IUserData {}
class UserDataB implements IUserData {}

class UserA extends FrameworkA {
  /* TB should fail because UserB has different TUserData
   * from the one defined here */
  const type TB = UserB;
  const type TUserData = UserDataA;
}

class UserB extends FrameworkB {
  const type TUserData = UserDataB;
}
