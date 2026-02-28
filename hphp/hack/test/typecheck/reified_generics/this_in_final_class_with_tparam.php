<?hh

type Id_of<+T> = int;

final class C<T> {
      // Even if we think of `this` in a final class as sugar
      // for the lexical class, C::Tc shouldn't be reifiable.
      // Consider that we also ban
      // const type Tc = Id_of<C<T>> since type parameters
      // are not allowed in type constants
      const type Tc = Id_of<this>;
}

function take_reified<reify T>(): void {}

function test(): void {
  take_reified<C::Tc>(); // Error
}
