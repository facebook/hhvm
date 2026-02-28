<?hh

class E<Ta, Tb> {}

interface I<
  Ta as arraykey,
  Tb as Ta,
  Tc as Td,
  Td as E<Ta, Tf>,
  Tf,
  Tarr as varray<Ta>,
  Tlike as ~Ta,
  Topt as ?Ta,
  Tfun as (function(Ta): Tb),
  Tshape as shape('a' => Ta),
  Tdarr as darray<Ta, Tb>,
  Tvarr as varray<Ta>,
  Tvdarr as varray_or_darray<Ta, Tb>,
  Ttuple as (Ta, Tb),
> {}
