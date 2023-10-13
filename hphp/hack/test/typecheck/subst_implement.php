<?hh

interface A {}

interface B extends A {}

interface Beep<T, T2 as T> {}

interface Boop extends Beep<A, B> {}
