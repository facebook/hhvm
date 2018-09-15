#define soit let
#define fonction function
#define fon fun
#define dans in
#define si if
#define alors then
#define sinon else

#define Liste List
#define Affichef Printf
#define affichef printf

#define separation split
#define tri sort

soit rec separation x = fonction
    y :: l ->
      soit l1, l2 = separation x l dans
      si y < x alors (y :: l1), l2
      sinon l1, (y :: l2)
  | [] ->
      [], []

soit rec tri = fonction
    x :: l ->
      soit l1, l2 = separation x l dans
      tri l1 @ [x] @ tri l2
  | [] ->
      []

soit () =
  soit l = tri [ 5; 3; 7; 1; 7; 4; 99; 22 ] dans
  Liste.iter (fon i -> Affichef.affichef "%i " i) l;
  Affichef.affichef "\n"
