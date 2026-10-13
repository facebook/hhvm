type a = int

type counters = {
  signed: int;
  unsigned: int; [@oxidize.usize]
}
