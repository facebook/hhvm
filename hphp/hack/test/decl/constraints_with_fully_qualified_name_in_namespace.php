<?hh

namespace {
  type Ta = int;
  type Te = string;
}

namespace NS {
  type Ta = int;
  type Te = string;

  interface I<
    Ta as arraykey,
    Tb as Map<Ta, Te>,
    Tc as Map<\Ta, \Te>,
    Td as Map<\NS\Ta, \NS\Te>,
    Te,
  >{}
}
