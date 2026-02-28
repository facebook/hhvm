<?hh

interface IBase {
  abstract const type T as arraykey;
}

interface I {
  const type T = string;
}

abstract class CBase implements IBase {}

abstract class C extends CBase implements I {}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
