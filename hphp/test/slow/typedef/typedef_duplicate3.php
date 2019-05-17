<?hh

class Existing {
}

type Existing = int; // error, Existing exists

<<__EntryPoint>> function main(): void {}
