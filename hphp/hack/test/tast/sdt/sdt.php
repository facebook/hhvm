<?hh

class C {}
interface I {}
trait T {}

function f(): void {}

// enums do not get SDT
enum E: int {}
