<?hh

enum CondEnum1 : int {
   VAL1 = 1;
   VAL2 = 2;
   VAL3 = 3;
}

type CondAlias = int;

const CONDCONST1 = 999;
const CONDCONST2 = 'zyzzy';
const CONDCONST3 = vec[1, 2, 3];
const CONDCONST4 = vec[1, 2, 3];
const CONDCONST5 = dict['a' => 100, 'b' => 200];
const CONDCONST6 = vec[1, 2, 3];

type VAlias = varray;
type DAlias = darray;
type AAlias = AnyArray;
