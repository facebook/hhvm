<?hh
<<file: __EnableUnstableFeatures('case_types')>>

interface MyInterface {}
case type MyCaseType = MyInterface | Awaitable<MyInterface>;
