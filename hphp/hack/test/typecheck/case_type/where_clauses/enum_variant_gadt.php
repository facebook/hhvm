<?hh

enum Color: string as string {
  RED = 'red';
  GREEN = 'green';
  BLUE = 'blue';
}

final class ColorResult {}
final class OtherResult {}

case type ColorOrInt<+T> =
  | Color where T super ColorResult
  | int where T super OtherResult;

case type ColorOrBool<+T> =
  | Color where T super ColorResult
  | bool where T super OtherResult;

function process_union_with_int<T>(ColorOrInt<T> $input): T {
  if ($input is Color) {
    hh_show($input);
    return
      new ColorResult(); // expect error because `is Enum` cannot exclude int
  } else {
    hh_show($input);
    return new OtherResult(); // no error
  }
}

function process_union_with_bool<T>(ColorOrBool<T> $input): T {
  if ($input is Color) {
    hh_show($input);
    return new ColorResult(); // no error
  } else {
    hh_show($input);
    return new OtherResult(); // no error
  }
}
