<?hh

class BaseWithPropA {
  private $hello = 0;
}

trait AHelloProperty {
  private $hello = 0;
}

class BaseWithTPropB {
    use AHelloProperty;
}

class SubclassA extends BaseWithPropA {
    use AHelloProperty;
}

class SubclassB extends BaseWithTPropB {
    use AHelloProperty;
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL | E_STRICT);

$a = new SubclassA;
var_dump($a);

$b = new SubclassB;
var_dump($b);
}
