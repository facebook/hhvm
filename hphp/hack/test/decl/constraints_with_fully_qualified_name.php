<?hh

type Ta = int;
type Td = string;

interface I<
  Ta as arraykey,
  Tb as Map<Ta, Td>,
  Tc as Map<\Ta, \Td>,
  Td,
>{}
