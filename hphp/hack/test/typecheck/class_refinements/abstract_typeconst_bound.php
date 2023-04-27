<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface IUserData {}

abstract class FrameworkA {
  abstract const type TB as FrameworkB with {type TUserData = this::TUserData};
  abstract const type TUserData as IUserData;

  public function getUserData() : this::TUserData {
    $userb_classname = type_structure(static::class, 'TB')['classname'];
    $userb = $userb_classname::make();
    return $userb->getUserData();
  }
}

abstract class FrameworkB {
  abstract const type TUserData as IUserData;

  abstract public static function make() : this;

  abstract public function getUserData() : this::TUserData;
}
