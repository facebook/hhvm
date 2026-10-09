<?hh

function returns_dict(): dict<int,
  int> {
  return "hello";
}

function no_args(): void {}

function call_with_too_many_args(): void {
  no_args(1,
    2);
}
