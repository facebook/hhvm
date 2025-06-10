<?hh

case type MutuallyRecursiveA = vec<MutuallyRecursiveB>;
case type MutuallyRecursiveB = dict<int, MutuallyRecursiveA>;

case type CaseTypeWithWhereClause<T> = int where int as MutuallyRecursiveA;
