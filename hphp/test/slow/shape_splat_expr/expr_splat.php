<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Expression-level shape splat: `shape(...$a, 'k' => v)` builds a dict at
// runtime via a rightmost-wins merge -- a later element (field or splat entry)
// overwrites an earlier duplicate key, keeping the original insertion order.
// Exercises the hackc emitter's iterator-copy lowering across interp/jit, and
// under repo mode confirms HHBBC needs no special handling (same bytecode).

function dump(string $label, mixed $v): void {
  echo $label;
  echo ": ";
  var_dump($v);
}

function get_shape(): shape('y' => bool, 'z' => float) {
  return shape('y' => true, 'z' => 3.14);
}

final class ShapeSplatProperty {
  public shape('x' => int, 'y' => string) $value =
    shape(...shape('x' => 1), 'y' => 'property');
}

function last_splat_value_is_released(): bool {
  $value = new stdClass();
  $weak_ref = new WeakRef($value);
  // The interpreter's bespoke conversion of a non-empty literal keeps the
  // original dict, and therefore $value, alive until request end. Insert the
  // value afterward so only references held by the splat affect this check.
  $source = shape();
  $source['value'] = $value;
  $value = null;

  shape(...$source);
  $source = shape();

  return !$weak_ref->valid();
}

function dynamic_splat_succeeds(dynamic $value): bool {
  try {
    $_ = shape(...$value);
    return true;
  } catch (Throwable $_) {
    return false;
  }
}

<<__EntryPoint>>
function main(): void {
  $a = shape('x' => 1, 'y' => 'hello');
  $b = shape('y' => true, 'z' => 3.14);
  $c = shape('z' => 'world');
  $empty = shape();

  // A single splat copies the operand.
  dump('only_splat', shape(...$a));

  // Two splats: 'y' from $b wins; 'z' contributed by $b. Order: x, y, z.
  dump('two_splats', shape(...$a, ...$b));

  // Three splats: 'z' from $c wins over $b.
  dump('three_splats', shape(...$a, ...$b, ...$c));

  // Interleaved fields and splats -- source order decides each key's winner.
  dump('interleaved', shape('x' => 100, ...$a, 'y' => 'override', ...$b, 'w' => 7));

  // A trailing field overrides a key contributed by an earlier splat.
  dump('trailing_field_wins', shape(...$b, 'y' => false));

  // A leading field is overridden by a later splat.
  dump('leading_field_overridden', shape('z' => 'first', ...$c));

  // Adjacent splats with an empty operand in the middle.
  dump('adjacent_with_empty', shape(...$a, ...$empty, ...$c));

  // Empty operand then a field.
  dump('empty_operand', shape(...$empty, 'only' => 1));

  // Splat of an empty shape only.
  dump('all_empty', shape(...$empty));

  // A nested shape value is preserved as-is.
  $n = shape('inner' => shape('a' => 1));
  dump('nested_value', shape(...$n, 'outer' => 2));

  // Non-literal operand (function-call result).
  dump('call_operand', shape(...$a, ...get_shape()));

  // Splat of an inline shape literal.
  dump('inline_splat', shape(...$a, ...shape('x' => 999, 'q' => 'new')));

  dump('property_initializer', (new ShapeSplatProperty())->value);
  dump('last_value_released', last_splat_value_is_released());
  dump('dynamic_dict', dynamic_splat_succeeds(dict['x' => 1]));
  dump('dynamic_int', dynamic_splat_succeeds(42));
  dump('dynamic_vec', dynamic_splat_succeeds(vec[1]));
  dump('dynamic_keyset', dynamic_splat_succeeds(keyset[1]));
  dump('dynamic_object', dynamic_splat_succeeds(new stdClass()));
}
