<?hh

enum class E : mixed {
  int X = 42;
  string A = 'zuck';
}

function f<T as int>(HH\EnumClass\Label<E, T> $label) : void {
}

function main(): void {
  f(#AUTO332 // During type checking: Label<E, Tvar(0)> knowing that  Tvar(0) <: int
             // During auto-completion: Label<E, Tvar(0)>, no constraints on Tvar(0)
}
