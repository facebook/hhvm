<?hh // experimental

abstract class Foo
{
  enum Field {
    case type T;
    case string ident;
    case T default_value;
  }

  public function get<TF from this::Field>(TF $f): TF::T
  {
    return static:@Field::default_value($f);
  }

  public function getID<TF from this::Field>(TF $f): string
  {
    return static:@Field::ident($f);
  }


  public static function introspect_fields() : void
  {
    foreach (static:@Field::Members() as $member) {
      \var_dump($member);
    }
  }
}

class Bar extends Foo
{
  enum Field {
    :@toto (
      type T = int,
      ident = "toto",
      default_value = 42
    );
  }

  public function Toto() : int {
    return $this->get(:@toto);
  }
}
