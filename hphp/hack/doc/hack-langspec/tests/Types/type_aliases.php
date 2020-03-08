<?hh // strict

require_once 'type_aliases_rf.php';

namespace NS_type_aliases;

// TTA == Transparent type alias
// OTA == Opaque type alias

newtype OTA1d = TTA1a;
type TTA1e = OTA1d;

// ------------------------------------------------------

function f1(TTA_n_bool $p1, TTA_n_bool $p2): void {
  $p1 = true;
  $p1 = null;
  $p1 = false;

  $p2 = true;
  $p2 = null;
  $p2 = false;
}

// ------------------------------------------------------

function f2(Counter $p1, Counter $p2): void {
  ++$p1;			// int-like operations are allowed because of Counter's constraint
  $v1 = $p1 + $p2;	// int-like operations are allowed because of Counter's constraint
}

// ------------------------------------------------------

class C_bool {
  public static function fa(bool $p1, TTA_bool1 $p2, TTA_bool2 $p3, TTA_bool3 $p4): void {}
  public static function fc(bool $p1, OTA_bool1 $p2, OTA_bool2 $p3, OTA_bool3 $p4): void {}
  public static function fb(bool $p1, TTA_bool1 $p2, OTA_bool1 $p3): void {
    C_bool::fa($p1, $p1, $p1, $p1);
    C_bool::fa($p2, $p2, $p2, $p2);
//    C_bool::fa($p3, $p3, $p3, $p3);	// all 4 are incompatible
//    C_bool::fc($p1, $p1, $p1, $p1);	// args 2, 3, 4 are incompatible
//    C_bool::fc($p2, $p2, $p2, $p2);	// args 2, 3, 4 are incompatible
//    C_bool::fc($p3, $p3, $p3, $p3);	// args 1, 3, 4 are incompatible
  }

// treat alias like a predefined type name: add ?, array<>, Vector<>

  public static function fz(?TTA_bool1 $p1, array<TTA_bool2> $p2, Vector<TTA_bool3> $p3): void {}
}

// ------------------------------------------------------

class C_enum_Modes {
  public static function fa(Modes $p1, TTA_enum_Modes $p2, OTA_enum_Modes $p3): void {}
  public static function fb(Modes $p1, TTA_enum_Modes $p2, OTA_enum_Modes $p3): void {
//    C_enum_Modes::fa($p1, $p1, $p1);	// 3rd arg is incompatible
//    C_enum_Modes::fa($p2, $p2, $p2);	// 3rd arg is incompatible
//    C_enum_Modes::fa($p3, $p3, $p3);	// 1st and 3rd args are incompatible
//    C_enum_Modes::fa(Modes::Stopped, Modes::Stopped, Modes::Stopped);	3rd arg is incompatible	}
/*
    C_enum_Modes::fa(
      TTA_enum_Modes::Stopped,	// all gag on xx::
      TTA_enum_Modes::Stopped,
      TTA_enum_Modes::Stopped
    );
*/
/*
    C_enum_Modes::fa(
      OTA_enum_Modes::Stopped,	// all gag on xx::
      OTA_enum_Modes::Stopped,
      OTA_enum_Modes::Stopped
    );
*/
  }
}

// ------------------------------------------------------

class C_arrays {
  public static function fa(TTA_array_string $p1, TTA_array_array_string $p2): void {}
  public static function fb(TTA_array_string $p1): void {
    C_arrays::fa($p1, array($p1, array('aa', 'bb')));
  }
}

// ------------------------------------------------------

class C_Fullname {
  private ?TTA_class_Fullname $pr = null;
  public static function fa(TTA_class_Fullname $p1, array<TTA_class_Fullname> $p2): void {
    $v = $p1->firstName;
    $v = $p2[0]->lastName;
  }
  
  public static function fb(OTA_class_Fullname $p1, array<OTA_class_Fullname> $p2): void {
//    $v = $p1->firstName;
//    $v = $p2[0]->lastName;
  }
}

// ------------------------------------------------------

class C_MyCollection {
  public static function fa(TTA_interface_MyCollection1 $p1, TTA_interface_MyCollection3 $p2): void {}
  public static function fb(TTA_interface_MyCollection1 $p1, TTA_interface_MyCollection3 $p2): void {
    C_MyCollection::fa($p1, $p2);
  }
}

// ------------------------------------------------------

class C_tuple {
  private ?TTA_tuple $pr = null;
  public static function fa(TTA_tuple $p1, array<TTA_tuple> $p2): void {}
}

// ------------------------------------------------------

class C_closure {
  private ?TTA_closure $pr = null;
  public static function fa(TTA_closure $p1, array<TTA_closure> $p2): void {
    $p1(123);
    $p2[0](123);
  }
}

// ------------------------------------------------------

function CYf1(TTA_shape_point $p1): void {}
function CYf2(): TTA_shape_point { return shape('x' => 10, 'y' => 12); }
class CY {
  private TTA_shape_point $p1 = shape('x' => 0, 'y' => 5);
}

function addComplex(TTA_shape_complex $c1, TTA_shape_complex $c2): TTA_shape_complex {
  $result = $c1;
  //	...
  return $result;
}

// ------------------------------------------------------

function main(): void {
//  C_bool::fb(true, true, true);	// 3rd arg incompatible
}

/* HH_FIXME[1002] call to main in strict*/
main();
