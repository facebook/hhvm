<?hh

enum CondEnum1 : string {
   VAL1 = 'VAL1';
   VAL2 = 'VAL2';
   VAL3 = 'VAL3';
}

type CondAlias = string;

const CONDCONST2 = 999;
const CONDCONST1 = 'zyzzy';
const CONDCONST3 = keyset[1, 2, 3];
const CONDCONST4 = vec[1, 2, 3];
const CONDCONST5 = dict['a' => 100, 'b' => 200];
const CONDCONST6 = vec[1, 2, 3];

type VAlias = vec;
type DAlias = dict;
type AAlias = keyset;
