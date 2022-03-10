<?hh

abstract enum class A: string {
    abstract string fred;
    abstract string ginger;
}

enum class E : string extends A {
    string fred = 'fred astaire';
    string ginger = 'ginger rogers';
}
