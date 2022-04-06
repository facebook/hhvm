<?hh

namespace NS0 {
  enum class EC: int {
    int A = 42;
  }
}

namespace {
  <<__EntryPoint>>
  function main(): void {
   echo \NS0\EC::nameOf(\NS0\EC#A). "\n";
  }
}
