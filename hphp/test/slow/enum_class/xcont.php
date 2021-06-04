<?hh
<<file:__EnableUnstableFeatures('enum_atom')>>

interface IXParam {
}

abstract class XParam<T> implements IXParam {
  private ?T $value = null;

  public function __construct()[] {}

  public function optional()[] : this {
    return $this;
  }

  public function defaultValue(T $data)[write_props] : this {
    $this->value = $data;
    return $this;
  }

  abstract public function coerceTo(mixed $data) : T;

}

class XString extends XParam<string> {
  <<__Override>>
    public function coerceTo(mixed $data) : string {
      return $data as string;
    }
}

class XInt extends XParam<int> {
  <<__Override>>
    public function coerceTo(mixed $data) : int {
      return $data as int;
    }
}

abstract final class X {
  public static function Int()[] : XInt {
    return new XInt();
  }

  public static function String()[] : XString {
    return new XString();
  }
}

enum class XE : IXParam {}

abstract class Controller {
  abstract const type TE as XE;

  private dict<string, mixed> $data = dict[];

  protected static function key<T, TParam as XParam<T>>(HH\Label<this::TE, TParam> $atom) : string {
    $ts = type_structure(static::class, 'TE');
    $cls = $ts['classname'] as nonnull;
    return $cls::nameOf($atom);
  }

  protected static function value<T, TParam as XParam<T>>(HH\Label<this::TE, TParam> $atom) : HH\MemberOf<this::TE, TParam> {
    $ts = type_structure(static::class, 'TE');
    $cls = $ts['classname'] as nonnull;
    return $cls::valueOf($atom);
  }

  public function set<T, TParam as XParam<T>>(HH\Label<this::TE, TParam> $atom, T $actual_data) : void {
    $key = static::key($atom);
    $this->data[$key] = $actual_data;
  }

  public function get<T, TParam as XParam<T>>(HH\Label<this::TE, TParam> $atom) : ?T {
    $key = static::key($atom);
    $definition = static::value($atom);

  // TODO: add the logic for defaultValue, but not important for this
  // demonstration
    $raw_data = $this->data[$key] ?? null;
    if ($raw_data is nonnull) {
      return $definition->coerceTo($raw_data);
    }
    return null;
  }
}

enum class SomeE : IXParam extends XE {
  XString Name = X::String()->defaultValue("zuck");
  XInt Age = X::Int()->defaultValue(42);
  XInt Foo = X::Int()->optional();
}

class SomeController extends Controller {
  const type TE = SomeE;
}

function expect_string(string $_): void {}
function expect_int(int $_): void {}

<<__EntryPoint>>
function main(): void {
  $cont = new SomeController();

  $cont->set#Name("zuck");
  $cont->set#Age(42);

  $name = $cont->get#Name();
  $age = $cont->get#Age();
  $foo = $cont->get#Foo();

  // $cont->get#XXX(); // Hack error

  if ($name is nonnull) {
    expect_string($name);
    echo "name = $name\n";
  } else {
    echo "no name\n";
  }

  if ($age is nonnull) {
    expect_int($age);
    echo "age = $age\n";
  } else {
    echo "no age\n";
  }

  if ($foo is nonnull) {
    expect_int($foo);
    echo "foo = $foo\n";
  } else {
    echo "no foo\n";
  }
}
