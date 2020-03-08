<?hh // strict

namespace NS_type_aliases;

// TTA == Transparent type alias
// OTA == Opaque type alias

// ------------------------------------------------------

type TTA1a = int;
newtype OTA1b = TTA1a;
type TTA1c = OTA1b;

// ------------------------------------------------------

type TTA_bool1 = bool;
newtype OTA_bool1 = bool;
type TTA_bool2 = bool;		// duplicate aliases
newtype OTA_bool2 = bool;	// duplicate aliases
type TTA_bool3 = TTA_bool1;	// aliasing an alias
newtype OTA_bool3 = OTA_bool1;	// aliasing an alias

// ------------------------------------------------------


/*
// Discovered to be no longer accepted as of Jan 18, 2016

type TTA_void = void;
newtype OTA_void = void;
*/

type TTA_int = int;
type TTA_float = float;
type TTA_num = num;
type TTA_string = string;
type TTA_arraykey = arraykey;
type TTA_resource = resource;

type TTA_n_bool = ?TTA_bool1;		// using a ? modifier + an alias
newtype OTA_n_bool = ?OTA_bool1;	// using a ? modifier + an alias

type TTA_n_int = ?int;
type TTA_n_n_int = ?TTA_n_int;		// I don't think this should be allowed

type TTA_mixed = mixed;
type TTA_n_mixed = ?TTA_mixed;		// I don't think this should be allowed

// ------------------------------------------------------

enum Modes: int { Stopped = 0; Stopping = 1; Starting = 2; Started = 3; }
type TTA_enum_Modes = Modes;
newtype OTA_enum_Modes = Modes;

// ------------------------------------------------------

type TTA_array_string = array<string>;
newtype OTA_array_string = array<string>;
type TTA_array_array_string = array<array<string>>;
newtype OTA_array_array_string = array<array<string>>;

// ------------------------------------------------------

class Fullname {
  public string $firstName = '';
  public string $lastName = '';
}
type TTA_class_Fullname = Fullname;
newtype OTA_class_Fullname = Fullname;

// ------------------------------------------------------

interface MyCollection<T> {
  public function put(T $item): void;
  public function get(): T;
}
type TTA_interface_MyCollection1 = MyCollection;
type TTA_interface_MyCollection2<T> = MyCollection<T>;	// Okay? Really?
type TTA_interface_MyCollection3 = MyCollection<int>;

// ------------------------------------------------------

trait TR1 { public function compute(): void {} }
trait TR2 { public function compute(): void {} }
trait TR3 { public function sort(): void {} }
trait TR4a {
  use TR3;
  use TR1, TR2;
}

/*
type TTA_trait1 = TR1;	// TR1 is uninstantiable
type TTA_trait2 = TR2;	// TR2 is uninstantiable
type TTA_trait3 = TR3;	// TR3 is uninstantiable
trait TR4b {
  use TTA_trait3;
  use TTA_trait1, TTA_trait2;
}
*/
// ------------------------------------------------------

type TTA_tuple = (int, string, int);
newtype OTA_tuple = (int, string, int);

// ------------------------------------------------------

type TTA_closure = (function (int): void);
newtype OTA_closure = (function (int): void);

// ------------------------------------------------------

type TTA_shape_point = shape('x' => int, 'y' => int);
newtype OTA_shape_point = shape('x' => int, 'y' => int);

type TTA_shape_complex = shape('real' => float, 'imag' => float);
newtype OTA_shape_complex = shape('real' => float, 'imag' => float);

// ------------------------------------------------------

newtype Counter as int = int;
//newtype Counter as float = int;	// float not compatible with int
//newtype Counter as num = int;
//newtype OTA_x1 as int = Fullname;	// int not compatible with Fullname

// ------------------------------------------------------
