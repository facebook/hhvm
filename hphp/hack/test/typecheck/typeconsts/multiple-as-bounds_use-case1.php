<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

newtype ID_of<+Truntime as arraykey, +Thint> as Truntime = Truntime;
newtype IntID_of<+Thint> as ID_of<int, Thint> = int;
function int_id(int $x)[]: IntID_of<nothing> { return $x; }

interface IHasID<+TID> {
  public function getID(): TID;
}

interface IBase extends IHasID<this::TID> {
  abstract const type TID as arraykey;
  <<__Override>>
  public function getID(): this::TID;
}

interface IRef2 {
  abstract const type TRefEnt as IBase;
  public function getID(): this::TRefEnt::TID;
}

interface IAnimalRef extends IRef2 {
  abstract const type TRefEnt as IAnimal;
}

// Problem: without multiple `as` bounds
//   IAnimalRef::getID() will return IAnimalRef::TRefEnt::TID
// But,
//   IAnimal::getID() will return this::TID
// and we get error "Expected this::TRefEnt::TID But got this::TID as IAnimal::TID
interface IAnimal extends IBase, IAnimalRef {
  abstract const type TID as int as this::TRefEnt::TID;
}

interface IDogRef extends IRef2, IAnimalRef {
  const type TRefEnt = Dog;
}

final class Dog implements IBase, IAnimal, IDogRef {
  const type TID = IntID_of<Dog>;
  public function getID(): IntID_of<Dog> {
    return int_id(5);
  }
}
