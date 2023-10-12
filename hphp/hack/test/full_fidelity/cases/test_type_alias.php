<?hh

type T1<T> = T2;
type T2 = T2;
type T3<T> = T3;
type T4<T, T> = T;
type T5<int, T> = T;
