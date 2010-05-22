<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

p(
<<<END
#include <sqlite3.h>
END
);

k("SQLITE3_ASSOC",          Int64);
k("SQLITE3_NUM",            Int64);
k("SQLITE3_BOTH",           Int64);
k("SQLITE3_INTEGER",        Int64);
k("SQLITE3_FLOAT",          Int64);
k("SQLITE3_TEXT",           Int64);
k("SQLITE3_BLOB",           Int64);
k("SQLITE3_NULL",           Int64);
k("SQLITE3_OPEN_READONLY",  Int64);
k("SQLITE3_OPEN_READWRITE", Int64);
k("SQLITE3_OPEN_CREATE",    Int64);

c('sqlite3', null, array(),
  array(
    m(PublicMethod, "__construct", null),
    m(PublicMethod, "open", null,
      array("filename" => String,
            "flags" => array(Int64,
                             "k_SQLITE3_OPEN_READWRITE|k_SQLITE3_OPEN_CREATE"),
            "encryption_key" => array(String, "null_string"))),
    m(PublicMethod, "close", Boolean),
    m(PublicMethod, "exec", Boolean,
      array("sql" => String)),
    m(PublicMethod, "version", VariantMap),
    m(PublicMethod, "lastInsertRowID", Int64),
    m(PublicMethod, "lastErrorCode", Int64),
    m(PublicMethod, "lastErrorMsg", String),
    m(PublicMethod, "loadExtension", Boolean,
      array("extension" => String)),
    m(PublicMethod, "changes", Int64),
    m(PublicMethod, "escapeString", String,
      array("sql" => String)),
    m(PublicMethod, "prepare", Variant,
      array("sql" => String)),
    m(PublicMethod, "query", Variant,
      array("sql" => String)),
    m(PublicMethod, "querySingle", Variant,
      array("sql" => String,
            "entire_row" => array(Boolean, "false"))),
    m(PublicMethod, "createFunction", Boolean,
      array("name" => String,
            "callback" => Variant,
            "argcount" => array(Int64, "-1"))),
    m(PublicMethod, "createAggregate", Boolean,
      array("name" => String,
            "step" => Variant,
            "final" => Variant,
            "argcount" => array(Int64, "-1"))),
    m(PublicMethod, "openBlob", Boolean,
      array("table" => String,
            "column" => String,
            "rowid" => Int64,
            "dbname" => array(String, "null_string"))),
  ),
  array(),
  "\n  public: void validate() const;".
  "\n  public: sqlite3 *m_raw_db;".
  "\n    DECLARE_BOOST_TYPES(UserDefinedFunc);".
  "\n    struct UserDefinedFunc {".
  "\n      int argc;".
  "\n      Variant func;".
  "\n      Variant step;".
  "\n      Variant fini;".
  "\n    };".
  "\n  public: UserDefinedFuncPtrVec m_udfs;"
 );

c('sqlite3stmt', null, array(),
  array(
    m(PublicMethod, "__construct", null,
      array("dbobject" => Object,
            "statement" => String)),
    m(PublicMethod, "paramCount", Int64),
    m(PublicMethod, "close", Boolean),
    m(PublicMethod, "reset", Boolean),
    m(PublicMethod, "clear", Boolean),
    m(PublicMethod, "bindParam", Boolean,
      array("name" => Variant,
            "parameter" => Variant | Reference,
            "type" => array(Int64, "k_SQLITE3_TEXT"))),
    m(PublicMethod, "bindValue", Boolean,
      array("name" => Variant,
            "parameter" => Variant,
            "type" => array(Int64, "k_SQLITE3_TEXT"))),
    m(PublicMethod, "execute", Variant),
  ),
  array(),
  "\n  public: void validate() const;".
  "\n  public: sp_sqlite3 m_db;".
  "\n  public: sqlite3_stmt *m_raw_stmt;".
  "\n    DECLARE_BOOST_TYPES(BoundParam);".
  "\n    struct BoundParam {".
  "\n      int type;".
  "\n      int index;".
  "\n      Variant value;".
  "\n    };".
  "\n  public: BoundParamPtrVec m_bound_params;"
 );

c('sqlite3result', null, array(),
  array(
    m(PublicMethod, "__construct", null),
    m(PublicMethod, "numColumns", Int64),
    m(PublicMethod, "columnName", String,
      array("column" => Int64)),
    m(PublicMethod, "columnType", Int64,
      array("column" => Int64)),
    m(PublicMethod, "fetchArray", Variant,
      array("mode" => array(Int64, "k_SQLITE3_BOTH"))),
    m(PublicMethod, "reset", Boolean),
    m(PublicMethod, "finalize", Boolean),
  ),
  array(),
  "\n  public: void validate() const;".
  "\n  public: sp_sqlite3stmt m_stmt;"
 );
