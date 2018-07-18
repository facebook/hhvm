<?hh

function Ref<T>(T $initial_value): Ref<T> {
  return new Ref($initial_value);
}

final class Ref<T> implements IRef<T> {

  public function __construct(public T $value) {}
}

function set_default<Tk as arraykey, Tv>(
  \Ref<dict<Tk, Tv>> $dict,
  Tk $key,
  Tv $default,
): \Ref<dict<Tk, Tv>> {
  if (!C\contains_key($dict->value, $key)) {
    $dict->value[$key] = $default;
  }
  return $dict;
}

function test(): void {
  $dict = Ref(dict[1 => Ref(vec['hi'])]);
  set_default($dict, 1, Ref(vec[]))->value[1]->value[] = 'bye';
}
