<?hh

enum class A:arraykey {
  int X = "a";  // decl type of X is \HH\MemberOf<A,int>
  string Y = 1; // decl type of Y is \HH\MemberOf<A,string>
}

enum B:int as arraykey {
  X = 1;       // decl type of X is int
  Y = "hello"; // decl type of Y is string
  Z = B::X;    // decl type of Z is TAny
}

const int I=1, S="a"; // decl type of I is int, S is int
const J=1, T="a";     // decl type of J is int, S is string

class C {
  const int IC = 1, SC = "a"; // decl type of IC is int, SC is int
  const JC=1, TC = "a";       // decl type of JC is int, TC is string
}
