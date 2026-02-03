<?hh <<file: __EnableUnstableFeatures('recursive_case_types')>>

case type MutuallyRecursiveA = vec<MutuallyRecursiveB>;
case type MutuallyRecursiveB = dict<int, MutuallyRecursiveA>;

case type CaseTypeWithWhereClause<T> = int where int as MutuallyRecursiveA;
