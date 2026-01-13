<?hh
<<file: __EnableUnstableFeatures('case_types', 'recursive_case_types')>>

final class Cov<+T> {}

final class Contra<-T> {}

case type A<+T> = Cov<A<T>>; // ok
case type B<-T> = Cov<B<T>>; // ok
case type D<-T> = Contra<D<T>>; // nok
case type E<+T> = Contra<E<T>>; // nok
