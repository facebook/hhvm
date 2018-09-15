<?hh


interface I {
  abstract const X;
  const Y = self::X . ' via Y';
}


<<__EntryPoint>>
function main_abstract_const6() {
var_dump(I::Y); // self::X cannot be resolved
}
