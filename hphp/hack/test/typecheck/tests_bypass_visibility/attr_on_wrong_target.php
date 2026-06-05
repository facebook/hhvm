<?hh

// What happens with the attribute on targets it shouldn't be on?

// On a top-level function (not allowed per contexts = [cls; mthd; instProperty; staticProperty])
<<__TestsBypassVisibility>>
function standalone(): void {}

// On an enum
<<__TestsBypassVisibility>>
enum MyEnum: int {}

// On a type alias
<<__TestsBypassVisibility>>
type MyAlias = int;
