<?hh

namespace HH\lmdb {
/*
enum MDB_ENV_FLAGS: int = {
  MDB_FIXEDMAP     =      0x01,
  MDB_NOSUBDIR     =    0x4000,
  MDB_NOSYNC       =   0x10000,
  MDB_RDONLY       =   0x20000,
  MDB_NOMETASYNC   =   0x40000,
  MDB_WRITEMAP     =   0x80000,
  MDB_MAPASYNC     =  0x100000,
  MDB_NOTLS        =  0x200000,
  MDB_NOLOCK       =  0x400000,
  MDB_NORDAHEAD    =  0x800000,
  MDB_NOMEMINIT    = 0x1000000,
  MDB_PREVSNAPSHOT = 0x2000000
};

enum MDB_DBI_FLAGS: int = {
  MDB_REVERSEKEY   =      0x02,
  MDB_DUPSORT      =      0x04,
  MDB_INTEGERKEY   =      0x08,
  MDB_DUPFIXED     =      0x10,
  MDB_INTEGERDUP   =      0x20,
  MDB_REVERSEDUP   =      0x40,
  MDB_CREATE       =   0x40000,
}

enum MDB_PUT_FLAGS: int = {
  MDB_NOOVERWRITE  =      0x10,
  MDB_NODUPDATA    =      0x20,
  MDB_CURRENT      =      0x40,
  MDB_RESERVE      =   0x10000,
  MDB_APPEND       =   0x20000,
  MDB_APPENDDUP    =   0x40000,
  MDB_MULTIPLE     =   0x80000,
}

enum MDB_COPY_FLAGS: int = {
  MDB_CP_COMPACT   =      0x01,
}

enum MDB_CURSOR_OP:int = {
  MDB_FIRST,
  MDB_FIRST_DUP,
  MDB_GET_BOTH,
  MDB_GET_BOTH_RANGE,
  MDB_GET_CURRENT,
  MDB_GET_MULTIPLE,
  MDB_LAST,
  MDB_LAST_DUP,
  MDB_NEXT,
  MDB_NEXT_DUP,
  MDB_NEXT_MULTIPLE,
  MDB_NEXT_NODUP,
  MDB_PREV,
  MDB_PREV_DUP,
  MDB_PREV_NODUP,
  MDB_SET,
  MDB_SET_KEY,
  MDB_SET_RANGE,
  MDB_PREV_MULTIPLE,
}

 */

enum LmdbKeyComparisonType: int {
  STRING = 0;
  STRING_NOCASE = 1;
  INTEGER = 2;
}

type MDB_env = shape (
  'env_handle' => int,
);

type MDB_dbi = shape (
  'db_handle' => int,
);

type MDB_txn = shape (
  'txn_handle' => int,
);

type MDB_stat = shape (
  'ms_psize' => int,
  'ms_depth' => int,
  'ms_branch_pages' => int,
  'ms_leaf_pages' => int,
  'ms_overflow_pages' => int,
  'ms_entries' => int,
);

type MDB_cursor = shape (
  'cursor_handle' => int,
);

type MDB_val = mixed;

<<__Native>>
function mdb_version(): (int, int, int);

<<__Native>>
function mdb_env_create(): /* MDB_env */ darray<string, dynamic>;

<<__Native>>
function mdb_env_open(/* MDB_env */ darray<string, dynamic> $env, string $path, int $flags, int $mode): void;

<<__Native>>
function mdb_env_close(/* MDB_env */ darray<string, dynamic> $env): void;

<<__Native>>
function mdb_env_set_mapsize(/* MDB_env */ darray<string, dynamic> $env, int $size): void;

<<__Native>>
function mdb_env_set_maxreaders(/* MDB_env */ darray<string, dynamic> $env, int $readers): void;

<<__Native>>
function mdb_env_set_maxdbs(/* MDB_env */ darray<string, dynamic> $env, int $dbs): void;

<<__Native>>
function mdb_env_stat(/* MDB_env */ darray<string, dynamic> $env): /* MDB_stat */ darray<string, dynamic>;

<<__Native>>
function mdb_env_get_path(/* MDB_env */ darray<string, dynamic> $env): string;

<<__Native>>
function mdb_env_get_maxkeysize(/* MDB_env */ darray<string, dynamic> $env): int;

<<__Native>>
function mdb_txn_begin(/* MDB_env */ darray<string, dynamic> $env, /* ?MDB_txn */ ?darray<string, dynamic> $parent, int $flags): /* MDB_txn */ darray<string, dynamic>;

<<__Native>>
function mdb_txn_commit(/* MDB_txn */ darray<string, dynamic> $txn): void;

<<__Native>>
function mdb_txn_abort(/* MDB_txn */ darray<string, dynamic> $txn): void;

<<__Native>>
function mdb_get(/* MDB_txn */ darray<string, dynamic> $txn, /* MDB_dbi */ darray<string, dynamic> $dbi, mixed /* MDB_val */ $key, inout mixed /* MDB_val */ $data): int;

<<__Native>>
function mdb_put(/* MDB_txn */ darray<string, dynamic> $txn, /* MDB_dbi */ darray<string, dynamic> $dbi, mixed $key, inout mixed $data, int $flags): int;

<<__Native>>
function mdb_del(/* MDB_txn */ darray<string, dynamic> $txn, /* MDB_dbi */ darray<string, dynamic> $dbi, mixed $key, mixed $data): int;

<<__Native>>
function mdb_reader_check(/* MDB_env */ darray<string, dynamic> $env): int;

<<__Native>>
function mdb_dbi_open(
  /* MDB_txn */               darray<string, dynamic> $txn,
  ?string $name,
  int $flags,
  /* LmdbKeyComparisonType */ int $key_comparison_type = 0,
): /* MDB_dbi */      darray<string, dynamic>;

<<__Native>>
function mdb_cursor_open(
  /* MDB_txn */       darray<string, dynamic> $txn,
  /* MDB_dbi */       darray<string, dynamic> $dbi
): /* MDB_cursor; */  darray<string, dynamic>;

<<__Native>>
function mdb_cursor_get(
  /* MDB_cursor */    darray<string, dynamic> $cursor,
  /* MDB_val */       inout mixed $key,
  /* MDB_val */       inout mixed $data,
  /* MDB_cursor_op */ int $op
): int;

<<__Native>>
function mdb_cursor_close(
  /* MDB_cursor */    darray<string, dynamic> $cursor
): void;

}
