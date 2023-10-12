<?hh // strict

abstract class XHPTest {
    public function __construct(
    public darray<string,mixed> $a, // Attributes
    public varray<mixed> $b, // Children
    public string $c, // Filename
    public int $d, // Line number
  ) { }
  }

class :x extends XHPTest { attribute int a @required; }
class :z extends XHPTest { attribute int a @required; attribute int b; }

function bar(bool $b, dynamic $d): void {
  // Effectively construct a like-type
  if ($b) {
        $a = <x a={1}/>;
  } else {
        $a = $d;
  }
  $c = <z {...$a} />;
}
