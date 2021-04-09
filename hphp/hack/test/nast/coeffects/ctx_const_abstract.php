<?hh

abstract class CWithConst {
  abstract const ctx C1; // bare abstract
  abstract const ctx C2 = [io]; // abstract with default
  abstract const ctx C3 as [io, rand]; // abstract with bound
  abstract const ctx C4 super [io, rand] as [io] = [io]; // abstract with multiple bounds and default
}
