<?hh

interface A<T1, T2> {}

interface B<T1> extends A<bool, T1> {}

interface C<T1> extends A<string, T1> {}

interface D extends B<float>, C<int> {}
