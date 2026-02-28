<?hh

class MyEnumClassKind {}
enum class MyEnumClass : MyEnumClassKind {
  MyEnumClassKind First = new MyEnumClassKind();
  MyEnumClassKind Second = new MyEnumClassKind();
}
