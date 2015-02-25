<?hh


interface I {
  abstract const X;
  const Y = self::X . ' via Y';
}

var_dump(I::Y); // self::X cannot be resolved
