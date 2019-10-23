<?hh

namespace MyNamespace {

    type MyString = string;
    type MyInt = int;
    type MyFloat = float;
    type MyNum = num;
    type MyBool = bool;

    namespace InnerNamespace {
        type MyInnerType = string;
    }

    namespace Very\Inner\Namespace {
        type MyVeryInnerNamespaceType = string;
    }

}
