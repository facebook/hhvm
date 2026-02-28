<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>


class D {
  const type TC1 = (function<T>(T): T);
  const type TC2 = (function((function<T>(T): T)): void);
  const type TC3 = (function(): (function<T>(T): T));
}
