<?hh


interface I {
  abstract const X;
  const Y = 'I::Y';
  const Z = self::Y . ' via Z';
}

var_dump(I::X); // no value!
