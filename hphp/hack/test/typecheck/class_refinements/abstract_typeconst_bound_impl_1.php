<?hh



interface IUserData {}

abstract class FrameworkA {
  abstract const type TB as FrameworkB with {type TUserData = this::TUserData};
  abstract const type TUserData as IUserData;
}

abstract class FrameworkB {
  abstract const type TUserData as IUserData;
}

class UserData implements IUserData {}

class UserA extends FrameworkA {
  const type TB = UserB;
  const type TUserData = UserData;
}

class UserB extends FrameworkB {
  const type TUserData = UserData;
}
