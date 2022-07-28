type a = {
  a_foo: int;
  a_bar: int;
}

type b =
  | X of {
      x_foo: int;
      x_bar: int;
    }
  | Y of {
      y_foo: int;
      y_bar: int;
    }
