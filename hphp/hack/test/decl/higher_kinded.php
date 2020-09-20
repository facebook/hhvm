<?hh // strict

type ID<T> = T;

type Test1<TC<TA1, TA2<TA3>>> = TC<int, ID>;
