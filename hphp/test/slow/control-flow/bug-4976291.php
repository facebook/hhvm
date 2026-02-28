<?hh

class E {}

class Dlgt extends E {
  private $ent;
  function __construct(E $e) {
    $this->ent = $e;
  }
}

class Dlgr {
  static function getDlgtObject($blah) :mixed{
    return $blah ? new Dlgt($blah) : nullptr;
  }
}

class X {
  private static $classCache = dict[];

  static function getuirc(E $blah) :mixed{
    $blah_class = get_class($blah);
    if (!array_key_exists($blah_class, self::$classCache)) {
      $blah_uric = null;
      for ($cls = $blah_class; $cls; $cls = get_parent_class($cls)) {
        $blah_uric = $cls.'URILoader';
        if (class_exists($blah_uric)) {
          break;
        }
      }

      if (is_subclass_of($blah_uric, Dlgr::class)) {
        $d = $blah_uric::getDlgtObject($blah);
        if ($d) {
          $blah_class = get_class($d);
          $blah_uric = self::getuirc($d);
        }
      }

      self::$classCache[$blah_class] =
        ($blah_uric !== __CLASS__ && class_exists($blah_uric)) ?
        $blah_uric :
        null;
    }
    return self::$classCache[$blah_class];
  }

}

class D1 extends Dlgt {}
class D1URILoader {}
class F extends E {}
class FURILoader extends Dlgr {
  static function getDlgtObject($blah) :mixed{
    return new D1($blah);
  }
}

class D2 extends Dlgt {}
class D2URILoader {}
class G extends E {}
class GURILoader extends Dlgr {
  static function getDlgtObject($blah) :mixed{
    return new D2($blah);
  }
}
class H extends G {}

function main($x) :mixed{
  X::getuirc($x);
}


<<__EntryPoint>>
function main_bug_4976291() :mixed{
for ($i=0; $i < 20; $i++) {
  main(new F);
  main(new H);
}

echo "Done\n";
}
