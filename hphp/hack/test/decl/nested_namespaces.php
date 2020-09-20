<?hh

namespace MyNamespace {

  type MyString = string;
  type MyInt = int;
  type MyFloat = float;
  type MyNum = num;
  type MyBool = bool;

  type MyNamespacedType = \MyNamespace\MyString;

  namespace InnerNamespace {
    type MyInnerType = string;
    type MyDoubleNamespacedType = \MyNamespace\MyNamespacedType;
  }

  namespace Very\Inner\Namespace {
    type MyVeryInnerNamespaceType = string;
    class MyClass {}
  }

  type MyVeryInnerNamespaceType = Very\Inner\Namespace\MyVeryInnerNamespaceType;
  const MyVeryInnerNamespaceType hello = "hello";
}
