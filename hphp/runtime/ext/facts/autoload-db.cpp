/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <atomic>
#include <chrono>
#include <exception>
#include <fcntl.h>
#include <grp.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

#include <folly/experimental/io/FsUtil.h>
#include <folly/json.h>
#include <folly/logging/xlog.h>

#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/util/assertions.h"
#include "hphp/util/hash.h"

namespace HPHP {
namespace Facts {

DBData::DBData(
    folly::fs::path path, SQLite::OpenMode rwMode, ::gid_t gid, ::mode_t perms)
    : m_path{std::move(path)}, m_rwMode{rwMode}, m_gid{gid}, m_perms{perms} {
  always_assert(m_path.is_absolute());

  // Coerce DB permissions into unix owner/group/other bits
  XLOGF(
      DBG1,
      "Coercing DB permission bits {:04o} to {:04o}",
      m_perms,
      (m_perms | 0600) & 0666);
  m_perms |= 0600;
  m_perms &= 0666;
}

bool DBData::operator==(const DBData& rhs) const {
  return m_path == rhs.m_path && m_rwMode == rhs.m_rwMode &&
         m_gid == rhs.m_gid && m_perms == rhs.m_perms;
}

std::string DBData::toString() const {
  return folly::sformat("DBData({}, {}, {})", m_path.native(), m_gid, m_perms);
}

size_t DBData::hash() const {
  return folly::hash::hash_combine(
      hash_string_cs(m_path.native().c_str(), m_path.native().size()),
      std::hash<gid_t>{}(m_gid),
      std::hash<mode_t>{}(m_perms));
}

namespace {

/**
 * Create the given file if it doesn't exist, setting its group ownership and
 * permissions along the way.
 */
void setFilePerms(const folly::fs::path& path, ::gid_t gid, ::mode_t perms) {
  XLOGF(
      DBG1,
      "Creating {} with gid={} and perms={:04o}",
      path.native(),
      gid,
      perms);
  int dbFd = ::open(path.native().c_str(), O_CREAT, perms);
  if (dbFd == -1) {
    XLOGF(ERR, "Could not open DB at {}: errno={}", path.native(), errno);
    return;
  }
  SCOPE_EXIT {
    ::close(dbFd);
  };
  if (::fchown(dbFd, -1, gid) == -1) {
    XLOGF(
        ERR,
        "Could not chown({}, -1, {}): errno={}",
        path.native(),
        gid,
        errno);
  }
  if (::fchmod(dbFd, perms) == -1) {
    XLOGF(
        ERR,
        "Could not chmod({}, {:04o}): errno={}",
        path.native(),
        perms,
        errno);
  }
}

// Representation of inheritance kinds in the DB

// `extends`, `implements`, or `use`
const int kDeriveKindExtends = 0;
const int kDeriveKindRequireExtends = 1;
const int kDeriveKindRequireImplements = 2;

constexpr int toDBEnum(DeriveKind kind) {
  switch (kind) {
    case DeriveKind::Extends:
      return kDeriveKindExtends;
    case DeriveKind::RequireExtends:
      return kDeriveKindRequireExtends;
    case DeriveKind::RequireImplements:
      return kDeriveKindRequireImplements;
  }
  return -1;
}

void createSchema(SQLiteTxn& txn) {
  // Basically copied wholesale from FlibAutoloadMapSQL.php in WWW.

  // Common DB

  // Parent tables
  txn.exec("CREATE TABLE IF NOT EXISTS all_paths ("
           " pathid INTEGER PRIMARY KEY,"
           " path TEXT NOT NULL UNIQUE"
           ")");

  // Table storing data about Classes, Interfaces, Enums, and Traits
  txn.exec("CREATE TABLE IF NOT EXISTS type_details ("
           " typeid INTEGER PRIMARY KEY,"
           " name TEXT NOT NULL COLLATE NOCASE,"
           " pathid INTEGER NOT NULL REFERENCES all_paths ON DELETE CASCADE,"
           " kind_of TEXT NOT NULL,"
           " flags INTEGER NOT NULL,"
           " UNIQUE (pathid, name)"
           ")");

  // Path assocs

  txn.exec(
      "CREATE TABLE IF NOT EXISTS path_sha1sum ("
      " pathid INTEGER NOT NULL UNIQUE REFERENCES all_paths ON DELETE CASCADE,"
      " sha1sum TEXT NOT NULL"
      ")");

  txn.exec("CREATE TABLE IF NOT EXISTS function_paths ("
           " pathid INTEGER NOT NULL REFERENCES all_paths ON DELETE CASCADE,"
           " function TEXT NOT NULL COLLATE NOCASE,"
           " UNIQUE (pathid, function)"
           ")");

  txn.exec("CREATE TABLE IF NOT EXISTS constant_paths ("
           " pathid INTEGER NOT NULL REFERENCES all_paths ON DELETE CASCADE,"
           " constant TEXT NOT NULL,"
           " UNIQUE (pathid, constant)"
           ")");

  txn.exec(
      "CREATE TABLE IF NOT EXISTS derived_types ("
      " derived_id INTEGER NOT NULL REFERENCES type_details ON DELETE CASCADE,"
      " base_name TEXT NOT NULL COLLATE NOCASE,"
      " kind INTEGER NOT NULL,"
      " UNIQUE (derived_id, base_name, kind)"
      ")");

  txn.exec("CREATE TABLE IF NOT EXISTS watchman ("
           " id INTEGER PRIMARY KEY CHECK (id = 0),"
           " clock TEXT NULL,"
           " mergebase TEXT NULL"
           ")");

  txn.exec("CREATE TABLE IF NOT EXISTS type_attributes ("
           " typeid INTEGER NOT NULL REFERENCES type_details ON DELETE CASCADE,"
           " attribute_name TEXT NOT NULL,"
           " attribute_position INTEGER NULL,"
           " attribute_value TEXT NULL,"
           " UNIQUE (typeid, attribute_name, attribute_position)"
           ")");

  txn.exec("CREATE TABLE IF NOT EXISTS method_attributes ("
           " typeid INTEGER NOT NULL REFERENCES type_details ON DELETE CASCADE,"
           " method TEXT NOT NULL,"
           " attribute_name TEXT NOT NULL,"
           " attribute_position INTEGER NULL,"
           " attribute_value TEXT NULL,"
           " UNIQUE (typeid, method, attribute_name, attribute_position)"
           ")");

  txn.exec("CREATE TABLE IF NOT EXISTS file_attributes ("
           " pathid INTEGER NOT NULL REFERENCES all_paths ON DELETE CASCADE,"
           " attribute_name TEXT NOT NULL,"
           " attribute_position INTEGER NULL,"
           " attribute_value TEXT NULL,"
           " UNIQUE (pathid, attribute_name, attribute_position)"
           ")");
}

void rebuildIndices(SQLiteTxn& txn) {

  // Basically copied wholesale from FlibAutoloadMapSQL.php in WWW.

  // type_details
  txn.exec("CREATE INDEX IF NOT EXISTS type_details__name"
           " ON type_details (name)");
  txn.exec("CREATE INDEX IF NOT EXISTS type_details__pathid"
           " ON type_details (pathid)");

  // function_paths
  txn.exec("CREATE INDEX IF NOT EXISTS function_paths__pathid"
           " ON function_paths (pathid)");
  txn.exec("CREATE INDEX IF NOT EXISTS function_paths__function"
           " ON function_paths (function)");

  // constant_paths
  txn.exec("CREATE INDEX IF NOT EXISTS constant_paths__pathid"
           " ON constant_paths (pathid)");
  txn.exec("CREATE INDEX IF NOT EXISTS constant_paths__constant"
           " ON constant_paths (constant)");

  // derived_types
  txn.exec("CREATE INDEX IF NOT EXISTS derived_types__base_name"
           " ON derived_types (base_name)");
  txn.exec("CREATE INDEX IF NOT EXISTS derived_types__derived_id"
           " ON derived_types (derived_id)");

  // type_attributes
  txn.exec("CREATE INDEX IF NOT EXISTS "
           "type_attributes__attribute_name__typeid__attribute_position"
           " ON type_attributes (attribute_name, typeid, attribute_position)");

  // method_attributes
  txn.exec(
      "CREATE INDEX IF NOT EXISTS "
      "method_attributes__attribute_name__typeid__method__attribute_position"
      " ON method_attributes ("
      "  attribute_name,"
      "  typeid,"
      "  method,"
      "  attribute_position"
      ")");

  // file_attributes
  txn.exec("CREATE INDEX IF NOT EXISTS "
           "file_attributes__attribute_name__pathid__attribute_position"
           " ON file_attributes (attribute_name, pathid, attribute_position)");
}

TypeKind toTypeKind(const std::string_view kind) {
  if (kind == kTypeKindClass) {
    return TypeKind::Class;
  } else if (kind == kTypeKindInterface) {
    return TypeKind::Interface;
  } else if (kind == kTypeKindEnum) {
    return TypeKind::Enum;
  } else if (kind == kTypeKindTrait) {
    return TypeKind::Trait;
  } else if (kind == kTypeKindTypeAlias) {
    return TypeKind::TypeAlias;
  } else {
    return TypeKind::Unknown;
  }
}

std::string getTransitiveDerivedTypesQueryStr(
    TypeKindMask kinds, DeriveKindMask deriveKinds) {
  auto typeKindWhereClause = [&]() -> std::string {
    if (kinds == kTypeKindAll || kinds == 0) {
      return "";
    }
    std::string clause = "AND (name=@base OR kind_of IN (";
    auto needsDelim = false;
    for (auto kind :
         {TypeKind::Class,
          TypeKind::Interface,
          TypeKind::Enum,
          TypeKind::Trait,
          TypeKind::TypeAlias}) {
      if (kinds & static_cast<int>(kind)) {
        if (needsDelim) {
          clause += ", ";
        } else {
          needsDelim = true;
        }
        clause += "\"";
        clause += toString(kind);
        clause += "\"";
      }
    }
    clause += "))";
    return clause;
  }();
  auto deriveKindWhereClause = [&]() -> std::string {
    if (deriveKinds == kDeriveKindAll || deriveKinds == 0) {
      return "";
    }
    bool needsDelim = false;
    std::string clause = "AND derived_types.kind IN (";
    for (auto deriveKind :
         {DeriveKind::Extends,
          DeriveKind::RequireExtends,
          DeriveKind::RequireImplements}) {
      if (deriveKinds & static_cast<int>(deriveKind)) {
        if (needsDelim) {
          clause += ", ";
        } else {
          needsDelim = true;
        }
        clause += folly::sformat("{}", toDBEnum(deriveKind));
      }
    }
    clause += ")";
    return clause;
  }();
  return folly::sformat(
      "WITH RECURSIVE subtypes(id) AS ("
      " SELECT typeid FROM type_details WHERE name=@base"
      " UNION SELECT derived_id FROM derived_types, type_details, subtypes"
      " WHERE derived_types.base_name=type_details.name"
      " AND type_details.typeid=subtypes.id"
      " {}"
      " {}"
      ")"
      " SELECT name, path, kind_of, flags FROM type_details"
      "  JOIN all_paths USING (pathid)"
      "  WHERE type_details.typeid IN (SELECT id FROM subtypes)"
      "  AND name <> @base"
      "  {}",
      typeKindWhereClause,
      deriveKindWhereClause,
      typeKindWhereClause);
}

struct PathStmts {
  explicit PathStmts(SQLite& db)
      : m_insert{db.prepare(
            "INSERT OR IGNORE INTO all_paths (path) VALUES (@path)")}
      , m_erase{db.prepare("DELETE FROM all_paths WHERE path = @path")}
      , m_getAll{db.prepare("SELECT path, sha1sum FROM path_sha1sum"
                            " JOIN all_paths USING (pathid)")} {
  }

  SQLiteStmt m_insert;
  SQLiteStmt m_erase;
  SQLiteStmt m_getAll;
};

struct Sha1HexStmts {
  explicit Sha1HexStmts(SQLite& db)
      : m_insert{db.prepare("INSERT OR REPLACE INTO path_sha1sum VALUES ("
                            " (SELECT pathid FROM all_paths WHERE path=@path),"
                            " @sha1sum"
                            ")")}
      , m_get{db.prepare("SELECT sha1sum FROM path_sha1sum"
                         " JOIN all_paths USING (pathid)"
                         " WHERE path = @path")} {
  }

  SQLiteStmt m_insert;
  SQLiteStmt m_get;
};

struct TypeStmts {
  explicit TypeStmts(SQLite& db)
      : m_insertDetails{db.prepare(
            "INSERT OR IGNORE INTO type_details (pathid, name, kind_of, "
            "flags)"
            " VALUES ("
            "  (SELECT pathid FROM all_paths WHERE path=@path),"
            "  @name,"
            "  @kind_of,"
            "  @flags"
            " )")}
      , m_getTypePath{db.prepare("SELECT path FROM type_details"
                                 " JOIN all_paths USING (pathid)"
                                 " WHERE name=@type")}
      , m_getPathTypes{db.prepare("SELECT name FROM type_details"
                                  " JOIN all_paths USING (pathid)"
                                  " WHERE path=@path")}
      , m_getKindAndFlags{db.prepare("SELECT kind_of, flags FROM type_details"
                                     " JOIN all_paths USING (pathid)"
                                     " WHERE name=@type"
                                     " AND path=@path")}
      , m_insertBaseType{db.prepare(
            "INSERT OR IGNORE INTO derived_types (base_name, derived_id, kind)"
            " VALUES ("
            "  @base,"
            "  (SELECT typeid FROM type_details JOIN all_paths USING (pathid)"
            "   WHERE name=@derived AND path=@path),"
            "  @kind"
            " )")}
      , m_getBaseTypes{db.prepare("SELECT base_name FROM derived_types"
                                  " JOIN type_details AS derived_type ON "
                                  "(derived_type.typeid=derived_id)"
                                  " JOIN all_paths USING (pathid)"
                                  " WHERE derived_type.name = @derived"
                                  " AND path = @path"
                                  " AND kind = @kind")}
      , m_getDerivedTypes{db.prepare(
            "SELECT path, derived_type.name FROM derived_types"
            " JOIN type_details AS derived_type ON "
            "(derived_type.typeid=derived_id)"
            " JOIN all_paths USING (pathid)"
            " WHERE base_name = @base"
            " AND kind = @kind")}
      , m_insertTypeAttribute{db.prepare(
            "INSERT OR IGNORE INTO type_attributes ("
            " typeid,"
            " attribute_name,"
            " attribute_position,"
            " attribute_value"
            ")"
            " VALUES ("
            "  (SELECT typeid FROM type_details JOIN all_paths USING (pathid)"
            "   WHERE name=@type AND path=@path),"
            "  @attribute_name,"
            "  @attribute_position,"
            "  @attribute_value"
            " )")}
      , m_insertMethodAttribute{db.prepare(
            "INSERT OR IGNORE INTO method_attributes ("
            " typeid,"
            " method,"
            " attribute_name,"
            " attribute_position,"
            " attribute_value"
            ")"
            " VALUES ("
            "  (SELECT typeid FROM type_details JOIN all_paths USING (pathid)"
            "   WHERE name=@type AND path=@path),"
            "  @method,"
            "  @attribute_name,"
            "  @attribute_position,"
            "  @attribute_value"
            " )")}
      , m_getTypeAttributes{db.prepare("SELECT DISTINCT attribute_name"
                                       " FROM type_attributes"
                                       "  JOIN type_details USING (typeid)"
                                       "  JOIN all_paths USING (pathid)"
                                       " WHERE name=@type AND path = @path")}
      , m_getMethodAttributes{db.prepare(
            "SELECT DISTINCT attribute_name"
            " FROM method_attributes"
            "  JOIN type_details USING (typeid)"
            "  JOIN all_paths USING (pathid)"
            " WHERE name=@type AND method=@method AND path = @path")}
      , m_getTypeAttributeArgs{db.prepare(
            "SELECT attribute_value"
            " FROM type_attributes"
            "  JOIN type_details USING (typeid)"
            "  JOIN all_paths USING (pathid)"
            " WHERE name = @type"
            " AND kind_of <> 'typeAlias'"
            " AND path = @path"
            " AND attribute_name = @attribute_name")}
      , m_getTypeAliasAttributeArgs{db.prepare(
            "SELECT attribute_value"
            " FROM type_attributes"
            "  JOIN type_details USING (typeid)"
            "  JOIN all_paths USING (pathid)"
            " WHERE name = @type"
            " AND kind_of = 'typeAlias'"
            " AND path = @path"
            " AND attribute_name = @attribute_name")}
      , m_getMethodAttributeArgs{db.prepare(
            "SELECT attribute_value"
            " FROM method_attributes"
            "  JOIN type_details USING (typeid)"
            "  JOIN all_paths USING (pathid)"
            " WHERE name = @type"
            " AND method = @method"
            " AND path = @path"
            " AND attribute_name = @attribute_name")}
      , m_getTypesWithAttribute{db.prepare(
            "SELECT name, path from type_details"
            " JOIN all_paths USING (pathid)"
            " WHERE EXISTS ("
            "  SELECT * FROM type_attributes"
            "   WHERE attribute_name = @attribute_name"
            "   AND type_attributes.typeid=type_details.typeid"
            " ) AND kind_of <> 'typeAlias'")}
      , m_getTypeAliasesWithAttribute{db.prepare(
            "SELECT name, path from type_details"
            " JOIN all_paths USING (pathid)"
            " WHERE EXISTS ("
            "  SELECT * FROM type_attributes"
            "   WHERE attribute_name = @attribute_name"
            "   AND type_attributes.typeid=type_details.typeid"
            " ) AND kind_of = 'typeAlias'")}
      , m_getMethodsInPath{db.prepare(
            "SELECT name, method, path FROM type_details"
            " JOIN method_attributes USING (typeid)"
            " JOIN all_paths USING (pathid) WHERE path = @path")}
      , m_getMethodsWithAttribute{db.prepare(
            "SELECT name, method, path"
            " FROM type_details"
            "  JOIN method_attributes USING (typeid)"
            "  JOIN all_paths USING (pathid)"
            " WHERE attribute_name = @attribute_name")}
      , m_getCorrectCase{db.prepare(
            "SELECT name FROM type_details WHERE name=@name")}
      , m_getAll{db.prepare("SELECT name, path from type_details JOIN "
                            "all_paths USING (pathid)")} {
  }

  SQLiteStmt m_insertDetails;
  SQLiteStmt m_getTypePath;
  SQLiteStmt m_getPathTypes;
  SQLiteStmt m_getKindAndFlags;
  SQLiteStmt m_insertBaseType;
  SQLiteStmt m_getBaseTypes;
  SQLiteStmt m_getDerivedTypes;
  SQLiteStmt m_insertTypeAttribute;
  SQLiteStmt m_insertMethodAttribute;
  SQLiteStmt m_getTypeAttributes;
  SQLiteStmt m_getMethodAttributes;
  SQLiteStmt m_getTypeAttributeArgs;
  SQLiteStmt m_getTypeAliasAttributeArgs;
  SQLiteStmt m_getMethodAttributeArgs;
  SQLiteStmt m_getTypesWithAttribute;
  SQLiteStmt m_getTypeAliasesWithAttribute;
  SQLiteStmt m_getMethodsInPath;
  SQLiteStmt m_getMethodsWithAttribute;
  SQLiteStmt m_getCorrectCase;
  SQLiteStmt m_getAll;
};

struct FileStmts {
  explicit FileStmts(SQLite& db)
      : m_insertFileAttribute{db.prepare(
            "INSERT OR IGNORE INTO file_attributes ("
            " pathid,"
            " attribute_name,"
            " attribute_position,"
            " attribute_value"
            ")"
            " VALUES ("
            "  (SELECT pathid FROM all_paths WHERE path=@path),"
            "  @attribute_name,"
            "  @attribute_position,"
            "  @attribute_value"
            " )")}
      , m_getFileAttributes{db.prepare("SELECT DISTINCT attribute_name"
                                       " FROM file_attributes"
                                       "  JOIN all_paths USING (pathid)"
                                       " WHERE path = @path")}
      , m_getFileAttributeArgs{db.prepare(
            "SELECT attribute_value"
            " FROM file_attributes"
            "  JOIN all_paths USING (pathid)"
            " WHERE path = @path"
            " AND attribute_name = @attribute_name")}
      , m_getFilesWithAttribute{
            db.prepare("SELECT path from all_paths"
                       " JOIN file_attributes USING (pathid)"
                       " WHERE attribute_name = @attribute_name")} {
  }
  SQLiteStmt m_insertFileAttribute;
  SQLiteStmt m_getFileAttributes;
  SQLiteStmt m_getFileAttributeArgs;
  SQLiteStmt m_getFilesWithAttribute;
};

struct FunctionStmts {
  explicit FunctionStmts(SQLite& db)
      : m_insert{db.prepare(
            "INSERT OR IGNORE INTO function_paths (function, pathid) VALUES ("
            " @function,"
            " (SELECT pathid FROM all_paths WHERE path=@path)"
            ")")}
      , m_getFunctionPath{db.prepare("SELECT path FROM function_paths"
                                     " JOIN all_paths USING (pathid)"
                                     " WHERE function=@function")}
      , m_getPathFunctions{db.prepare("SELECT function FROM function_paths"
                                      " JOIN all_paths USING (pathid)"
                                      " WHERE path=@path")}
      , m_getCorrectCase{db.prepare(
            "SELECT function from function_paths where function=@function")}
      , m_getAll{db.prepare("SELECT function, path FROM function_paths JOIN "
                            "all_paths USING (pathid)")} {
  }

  SQLiteStmt m_insert;
  SQLiteStmt m_getFunctionPath;
  SQLiteStmt m_getPathFunctions;
  SQLiteStmt m_getCorrectCase;
  SQLiteStmt m_getAll;
};

struct ConstantStmts {
  explicit ConstantStmts(SQLite& db)
      : m_insert{db.prepare(
            "INSERT OR IGNORE INTO constant_paths (constant, pathid) VALUES("
            " @constant,"
            " (SELECT pathid FROM all_paths"
            "  WHERE path=@path)"
            ")")}
      , m_getConstantPath{db.prepare("SELECT path FROM constant_paths"
                                     " JOIN all_paths USING (pathid)"
                                     " WHERE constant=@constant")}
      , m_getPathConstants{db.prepare("SELECT constant FROM constant_paths"
                                      " JOIN all_paths USING (pathid)"
                                      " WHERE path=@path")}
      , m_getAll{db.prepare("SELECT constant, path FROM constant_paths JOIN "
                            "all_paths USING (pathid)")} {
  }

  SQLiteStmt m_insert;
  SQLiteStmt m_getConstantPath;
  SQLiteStmt m_getPathConstants;
  SQLiteStmt m_getAll;
};

struct ClockStmts {
  explicit ClockStmts(SQLite& db)
      : m_insert{db.prepare(
            "INSERT OR REPLACE INTO watchman (OID, clock, mergebase)"
            " VALUES (0, @clock, @mergebase)")}
      , m_get{db.prepare("SELECT clock, mergebase FROM watchman WHERE OID=0")} {
  }
  SQLiteStmt m_insert;
  SQLiteStmt m_get;
};

struct AutoloadDBImpl final : public AutoloadDB {
  explicit AutoloadDBImpl(SQLite db)
      : m_db{std::move(db)}
      , m_pathStmts{m_db}
      , m_sha1HexStmts{m_db}
      , m_typeStmts{m_db}
      , m_fileStmts{m_db}
      , m_functionStmts{m_db}
      , m_constantStmts{m_db}
      , m_clockStmts{m_db} {
  }

  AutoloadDBImpl(const AutoloadDBImpl&) = delete;
  AutoloadDBImpl(AutoloadDBImpl&&) noexcept = default;
  AutoloadDBImpl& operator=(const AutoloadDBImpl&) = delete;
  AutoloadDBImpl& operator=(AutoloadDBImpl&&) noexcept = delete;

  ~AutoloadDBImpl() override = default;

  static AutoloadDBImpl get(const DBData& dbData) {
    assertx(dbData.m_path.is_absolute());
    auto db = [&]() {
      try {
        return SQLite::connect(dbData.m_path.native(), dbData.m_rwMode);
      } catch (SQLiteExc& e) {
        XLOGF(
            ERR,
            "Exception when trying to openo/create native Facts DB at {} ({})",
            dbData.m_path.native(),
            e.what());
        throw std::runtime_error{folly::sformat(
            "Couldn't open or create native Facts DB at {}",
            dbData.m_path.native())};
      }
    }();

    if (dbData.m_rwMode == SQLite::OpenMode::ReadWrite) {
      // If writable, ensure the DB has the correct owner and permissions.
      setFilePerms(dbData.m_path, dbData.m_gid, dbData.m_perms);
      setFilePerms(
          folly::fs::path{dbData.m_path} += "-shm",
          dbData.m_gid,
          dbData.m_perms);
      setFilePerms(
          folly::fs::path{dbData.m_path} += "-wal",
          dbData.m_gid,
          dbData.m_perms);
    }

    if (!db.isReadOnly()) {
      try {
        db.setJournalMode(SQLite::JournalMode::WAL);
      } catch (SQLiteExc& e) {
        switch (e.code()) {
          case SQLiteExc::Code::BUSY:
            // This happens if multiple connections attempt to set WAL mode at
            // the same time. We only need one connection to succeed.
            break;
          default:
            throw;
        }
      }
    }

    db.setSynchronousLevel(SQLite::SynchronousLevel::OFF);
    {
      auto txn = db.begin();
      createSchema(txn);
      rebuildIndices(txn);
      db.setBusyTimeout(60'000);
      txn.commit();
      XLOGF(INFO, "Connected to SQLite DB at {}", dbData.m_path.native());
    }

    return AutoloadDBImpl{std::move(db)};
  }

  SQLiteTxn begin() override {
    return m_db.begin();
  }

  void insertPath(SQLiteTxn& txn, const folly::fs::path& path) override {
    assertx(path.is_relative());
    XLOGF(DBG9, "Registering path {} in the DB", path.native());
    auto query = txn.query(m_pathStmts.m_insert);
    query.bindString("@path", path.native());
    XLOGF(DBG9, "Running {}", query.sql());
    query.step();
  }

  void insertSha1Hex(
      SQLiteTxn& txn,
      const folly::fs::path& path,
      const Optional<std::string>& sha1hex) override {
    assertx(path.is_relative());
    auto query = txn.query(m_sha1HexStmts.m_insert);
    query.bindString("@path", path.native());
    if (sha1hex) {
      query.bindString("@sha1sum", *sha1hex);
    } else {
      query.bindNull("@sha1sum");
    }
    XLOGF(DBG9, "Running {}", query.sql());
    query.step();
  }

  std::string getSha1Hex(SQLiteTxn& txn, const folly::fs::path& path) override {
    assertx(path.is_relative());
    auto query = txn.query(m_sha1HexStmts.m_get);
    query.bindString("@path", path.native());
    XLOGF(DBG9, "Running {}", query.sql());
    query.step();
    return std::string{query.getString(0)};
  }

  void erasePath(SQLiteTxn& txn, const folly::fs::path& path) override {
    assertx(path.is_relative());
    auto query = txn.query(m_pathStmts.m_erase);
    query.bindString("@path", path.native());
    XLOGF(DBG9, "Running {}", query.sql());
    query.step();
  }

  void insertType(
      SQLiteTxn& txn,
      std::string_view type,
      const folly::fs::path& path,
      TypeKind kind,
      int flags) override {
    assertx(path.is_relative());
    {
      auto query = txn.query(m_typeStmts.m_insertDetails);
      query.bindString("@name", type);
      query.bindString("@path", path.native());
      query.bindString("@kind_of", toString(kind));
      query.bindInt("@flags", flags);
      XLOGF(DBG9, "Running {}", query.sql());
      query.step();
    }
  }

  std::vector<folly::fs::path>
  getTypePath(SQLiteTxn& txn, std::string_view type) override {
    auto query = txn.query(m_typeStmts.m_getTypePath);
    query.bindString("@type", type);
    std::vector<folly::fs::path> results;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      results.emplace_back(std::string{query.getString(0)});
    }
    return results;
  }

  std::vector<std::string>
  getPathTypes(SQLiteTxn& txn, const folly::fs::path& path) override {
    assertx(path.is_relative());
    auto query = txn.query(m_typeStmts.m_getPathTypes);
    query.bindString("@path", path.native());
    std::vector<std::string> types;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      types.emplace_back(query.getString(0));
    }
    return types;
  }

  std::pair<TypeKind, int> getKindAndFlags(
      SQLiteTxn& txn,
      const std::string_view type,
      const folly::fs::path& path) override {
    auto query = txn.query(m_typeStmts.m_getKindAndFlags);
    query.bindString("@type", type);
    query.bindString("@path", path.native());
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      return {toTypeKind(query.getString(0)), query.getInt(1)};
    }
    return {TypeKind::Unknown, 0};
  }

  void insertBaseType(
      SQLiteTxn& txn,
      const folly::fs::path& path,
      const std::string_view derived,
      DeriveKind kind,
      const std::string_view base) override {
    assertx(path.is_relative());
    auto query = txn.query(m_typeStmts.m_insertBaseType);
    query.bindString("@path", path.native());
    query.bindString("@derived", derived);
    query.bindInt("@kind", toDBEnum(kind));
    query.bindString("@base", base);
    XLOGF(DBG9, "Running {}", query.sql());
    query.step();
  }

  std::vector<std::string> getBaseTypes(
      SQLiteTxn& txn,
      const folly::fs::path& path,
      const std::string_view derived,
      DeriveKind kind) override {
    assertx(path.is_relative());
    auto query = txn.query(m_typeStmts.m_getBaseTypes);
    query.bindString("@derived", derived);
    query.bindString("@path", path.native());
    query.bindInt("@kind", toDBEnum(kind));
    std::vector<std::string> types;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      types.emplace_back(query.getString(0));
    }
    return types;
  }

  std::vector<std::pair<folly::fs::path, std::string>> getDerivedTypes(
      SQLiteTxn& txn, const std::string_view base, DeriveKind kind) override {
    auto query = txn.query(m_typeStmts.m_getDerivedTypes);
    query.bindString("@base", base);
    query.bindInt("@kind", toDBEnum(kind));
    std::vector<std::pair<folly::fs::path, std::string>> edges;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      edges.push_back(
          {folly::fs::path{std::string{query.getString(0)}},
           std::string{query.getString(1)}});
    }
    return edges;
  }

  SQLiteStmt& getTransitiveDerivedTypesStmt(
      TypeKindMask kinds, DeriveKindMask deriveKinds) {
    auto it = m_derivedTypeStmts.find({kinds, deriveKinds});
    if (it == m_derivedTypeStmts.end()) {
      return m_derivedTypeStmts
          .insert(
              {std::make_tuple(kinds, deriveKinds),
               m_db.prepare(
                   getTransitiveDerivedTypesQueryStr(kinds, deriveKinds))})
          .first->second;
    }
    return it->second;
  }

  RowIter<DerivedTypeInfo> getTransitiveDerivedTypes(
      SQLiteTxn& txn,
      const std::string_view baseType,
      TypeKindMask kinds = kTypeKindAll,
      DeriveKindMask deriveKinds = kDeriveKindAll) override {
    auto query = txn.query(getTransitiveDerivedTypesStmt(kinds, deriveKinds));
    query.bindString("@base", baseType);
    XLOGF(DBG9, "Running {}", query.sql());
    return RowIter<DerivedTypeInfo>{
        std::move(query), [](SQLiteQuery& query) -> DerivedTypeInfo {
          return {
              query.getString(0),
              query.getString(1),
              toTypeKind(query.getString(2)),
              query.getInt(3)};
        }};
  }

  void insertTypeAttribute(
      SQLiteTxn& txn,
      const folly::fs::path& path,
      const std::string_view type,
      const std::string_view attributeName,
      Optional<int> attributePosition,
      const folly::dynamic* attributeValue) override {

    std::string attrValueJson;
    auto query = txn.query(m_typeStmts.m_insertTypeAttribute);

    auto const attributeValueKey = "@attribute_value";
    if (attributeValue) {
      attrValueJson = folly::toJson(*attributeValue);
      query.bindString(attributeValueKey, attrValueJson);
    } else {
      query.bindNull(attributeValueKey);
    }

    auto const attributePositionKey = "@attribute_position";
    if (attributePosition) {
      query.bindInt(attributePositionKey, *attributePosition);
    } else {
      query.bindNull(attributePositionKey);
    }

    query.bindString("@type", type);
    query.bindString("@path", path.native());
    query.bindString("@attribute_name", attributeName);

    XLOGF(DBG9, "Running {}", query.sql());
    query.step();
  }

  void insertMethodAttribute(
      SQLiteTxn& txn,
      const folly::fs::path& path,
      std::string_view type,
      std::string_view method,
      std::string_view attributeName,
      Optional<int> attributePosition,
      const folly::dynamic* attributeValue) override {

    std::string attrValueJson;
    auto query = txn.query(m_typeStmts.m_insertMethodAttribute);

    auto const attributeValueKey = "@attribute_value";
    if (attributeValue) {
      attrValueJson = folly::toJson(*attributeValue);
      query.bindString(attributeValueKey, attrValueJson);
    } else {
      query.bindNull(attributeValueKey);
    }

    auto const attributePositionKey = "@attribute_position";
    if (attributePosition) {
      query.bindInt(attributePositionKey, *attributePosition);
    } else {
      query.bindNull(attributePositionKey);
    }

    query.bindString("@type", type);
    query.bindString("@method", method);
    query.bindString("@path", path.native());
    query.bindString("@attribute_name", attributeName);

    XLOGF(DBG9, "Running {}", query.sql());
    query.step();
  }

  void insertFileAttribute(
      SQLiteTxn& txn,
      const folly::fs::path& path,
      std::string_view attributeName,
      Optional<int> attributePosition,
      const folly::dynamic* attributeValue) override {

    std::string attrValueJson;
    auto query = txn.query(m_fileStmts.m_insertFileAttribute);

    auto const attributeValueKey = "@attribute_value";
    if (attributeValue) {
      attrValueJson = folly::toJson(*attributeValue);
      query.bindString(attributeValueKey, attrValueJson);
    } else {
      query.bindNull(attributeValueKey);
    }

    auto const attributePositionKey = "@attribute_position";
    if (attributePosition) {
      query.bindInt(attributePositionKey, *attributePosition);
    } else {
      query.bindNull(attributePositionKey);
    }

    query.bindString("@path", path.native());
    query.bindString("@attribute_name", attributeName);

    XLOGF(DBG9, "Running {}", query.sql());
    query.step();
  }

  void analyze() override {
    m_db.analyze();
  }

  std::vector<std::string> getAttributesOfType(
      SQLiteTxn& txn,
      const std::string_view type,
      const folly::fs::path& path) override {
    auto query = txn.query(m_typeStmts.m_getTypeAttributes);
    query.bindString("@type", type);
    query.bindString("@path", path.native());
    std::vector<std::string> results;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      results.emplace_back(query.getString(0));
    }
    return results;
  }

  std::vector<std::string> getAttributesOfMethod(
      SQLiteTxn& txn,
      std::string_view type,
      std::string_view method,
      const folly::fs::path& path) override {
    auto query = txn.query(m_typeStmts.m_getMethodAttributes);
    query.bindString("@type", type);
    query.bindString("@method", method);
    query.bindString("@path", path.native());
    std::vector<std::string> results;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      results.emplace_back(query.getString(0));
    }
    return results;
  }

  std::vector<std::string>
  getAttributesOfFile(SQLiteTxn& txn, const folly::fs::path& path) override {
    auto query = txn.query(m_fileStmts.m_getFileAttributes);
    query.bindString("@path", path.native());
    std::vector<std::string> results;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      results.emplace_back(query.getString(0));
    }
    return results;
  }

  std::vector<TypeDeclaration> getTypesWithAttribute(
      SQLiteTxn& txn, const std::string_view attributeName) override {
    auto query = txn.query(m_typeStmts.m_getTypesWithAttribute);
    query.bindString("@attribute_name", attributeName);
    std::vector<TypeDeclaration> results;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      results.push_back(TypeDeclaration{
          .m_type = std::string{query.getString(0)},
          .m_path = folly::fs::path{std::string{query.getString(1)}}});
    }
    return results;
  }

  std::vector<TypeDeclaration> getTypeAliasesWithAttribute(
      SQLiteTxn& txn, const std::string_view attributeName) override {
    auto query = txn.query(m_typeStmts.m_getTypeAliasesWithAttribute);
    query.bindString("@attribute_name", attributeName);
    std::vector<TypeDeclaration> results;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      results.push_back(TypeDeclaration{
          .m_type = std::string{query.getString(0)},
          .m_path = folly::fs::path{std::string{query.getString(1)}}});
    }
    return results;
  }

  std::vector<MethodDeclaration>
  getPathMethods(SQLiteTxn& txn, std::string_view path) override {
    auto query = txn.query(m_typeStmts.m_getMethodsInPath);
    query.bindString("@path", path);
    std::vector<MethodDeclaration> results;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      results.push_back(MethodDeclaration{
          .m_type = std::string{query.getString(0)},
          .m_method = std::string{query.getString(1)},
          .m_path = folly::fs::path{std::string{query.getString(2)}}});
    }
    return results;
  }

  std::vector<MethodDeclaration> getMethodsWithAttribute(
      SQLiteTxn& txn, std::string_view attributeName) override {
    auto query = txn.query(m_typeStmts.m_getMethodsWithAttribute);
    query.bindString("@attribute_name", attributeName);
    std::vector<MethodDeclaration> results;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      results.push_back(MethodDeclaration{
          .m_type = std::string{query.getString(0)},
          .m_method = std::string{query.getString(1)},
          .m_path = folly::fs::path{std::string{query.getString(2)}}});
    }
    return results;
  }

  std::vector<folly::fs::path> getFilesWithAttribute(
      SQLiteTxn& txn, const std::string_view attributeName) override {
    auto query = txn.query(m_fileStmts.m_getFilesWithAttribute);
    query.bindString("@attribute_name", attributeName);
    std::vector<folly::fs::path> results;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      results.push_back(folly::fs::path{std::string{query.getString(0)}});
    }
    return results;
  }

  std::vector<folly::dynamic> getTypeAttributeArgs(
      SQLiteTxn& txn,
      const std::string_view type,
      const std::string_view path,
      const std::string_view attributeName) override {
    auto query = txn.query(m_typeStmts.m_getTypeAttributeArgs);
    query.bindString("@type", type);
    query.bindString("@path", path);
    query.bindString("@attribute_name", attributeName);
    XLOGF(DBG9, "Running {}", query.sql());
    std::vector<folly::dynamic> args;
    for (query.step(); query.row(); query.step()) {
      auto arg = query.getNullableString(0);
      if (arg) {
        args.push_back(folly::parseJson(*arg));
      }
    }
    return args;
  }

  std::vector<folly::dynamic> getTypeAliasAttributeArgs(
      SQLiteTxn& txn,
      const std::string_view typeAlias,
      const std::string_view path,
      const std::string_view attributeName) override {
    auto query = txn.query(m_typeStmts.m_getTypeAliasAttributeArgs);
    query.bindString("@type", typeAlias);
    query.bindString("@path", path);
    query.bindString("@attribute_name", attributeName);
    XLOGF(DBG9, "Running {}", query.sql());
    std::vector<folly::dynamic> args;
    for (query.step(); query.row(); query.step()) {
      auto arg = query.getNullableString(0);
      if (arg) {
        args.push_back(folly::parseJson(*arg));
      }
    }
    return args;
  }

  std::vector<folly::dynamic> getMethodAttributeArgs(
      SQLiteTxn& txn,
      std::string_view type,
      std::string_view method,
      std::string_view path,
      std::string_view attributeName) override {
    auto query = txn.query(m_typeStmts.m_getMethodAttributeArgs);
    query.bindString("@type", type);
    query.bindString("@method", method);
    query.bindString("@path", path);
    query.bindString("@attribute_name", attributeName);
    XLOGF(DBG9, "Running {}", query.sql());
    std::vector<folly::dynamic> args;
    for (query.step(); query.row(); query.step()) {
      auto arg = query.getNullableString(0);
      if (arg) {
        args.push_back(folly::parseJson(*arg));
      }
    }
    return args;
  }

  std::vector<folly::dynamic> getFileAttributeArgs(
      SQLiteTxn& txn,
      const std::string_view path,
      const std::string_view attributeName) override {
    auto query = txn.query(m_fileStmts.m_getFileAttributeArgs);
    query.bindString("@path", path);
    query.bindString("@attribute_name", attributeName);
    XLOGF(DBG9, "Running {}", query.sql());
    std::vector<folly::dynamic> args;
    for (query.step(); query.row(); query.step()) {
      auto arg = query.getNullableString(0);
      if (arg) {
        args.push_back(folly::parseJson(*arg));
      }
    }
    return args;
  }

  std::string
  getTypeCorrectCase(SQLiteTxn& txn, std::string_view type) override {
    auto query = txn.query(m_typeStmts.m_getCorrectCase);
    query.bindString("@name", type);
    for (query.step(); query.row(); query.step()) {
      return std::string{query.getString(0)};
    }
    return {};
  }

  void insertFunction(
      SQLiteTxn& txn,
      std::string_view function,
      const folly::fs::path& path) override {
    assertx(path.is_relative());
    auto query = txn.query(m_functionStmts.m_insert);
    query.bindString("@function", function);
    query.bindString("@path", path.native());
    XLOGF(DBG9, "Running {}", query.sql());
    query.step();
  }

  std::vector<folly::fs::path>
  getFunctionPath(SQLiteTxn& txn, std::string_view function) override {
    auto query = txn.query(m_functionStmts.m_getFunctionPath);
    query.bindString("@function", function);
    XLOGF(DBG9, "Running {}", query.sql());
    std::vector<folly::fs::path> results;
    for (query.step(); query.row(); query.step()) {
      results.emplace_back(std::string{query.getString(0)});
    }
    return results;
  }

  std::vector<std::string>
  getPathFunctions(SQLiteTxn& txn, const folly::fs::path& path) override {
    assertx(path.is_relative());
    auto query = txn.query(m_functionStmts.m_getPathFunctions);
    query.bindString("@path", path.native());
    std::vector<std::string> functions;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      functions.emplace_back(query.getString(0));
    }
    return functions;
  }

  std::string
  getFunctionCorrectCase(SQLiteTxn& txn, std::string_view function) override {
    auto query = txn.query(m_functionStmts.m_getCorrectCase);
    query.bindString("@function", function);
    for (query.step(); query.row(); query.step()) {
      return std::string{query.getString(0)};
    }
    return {};
  }

  void insertConstant(
      SQLiteTxn& txn,
      std::string_view constant,
      const folly::fs::path& path) override {
    assertx(path.is_relative());
    auto query = txn.query(m_constantStmts.m_insert);
    query.bindString("@constant", constant);
    query.bindString("@path", path.native());
    XLOGF(DBG9, "Running {}", query.sql());
    query.step();
  }

  std::vector<folly::fs::path>
  getConstantPath(SQLiteTxn& txn, std::string_view constant) override {
    auto query = txn.query(m_constantStmts.m_getConstantPath);
    query.bindString("@constant", constant);
    std::vector<folly::fs::path> results;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      results.emplace_back(std::string{query.getString(0)});
    }
    return results;
  }

  std::vector<std::string>
  getPathConstants(SQLiteTxn& txn, const folly::fs::path& path) override {
    assertx(path.is_relative());
    auto query = txn.query(m_constantStmts.m_getPathConstants);
    query.bindString("@path", path.native());
    std::vector<std::string> constants;
    XLOGF(DBG9, "Running {}", query.sql());
    for (query.step(); query.row(); query.step()) {
      constants.emplace_back(query.getString(0));
    }
    return constants;
  }

  RowIter<PathAndHash> getAllPathsAndHashes(SQLiteTxn& txn) override {
    auto query = txn.query(m_pathStmts.m_getAll);
    XLOGF(DBG9, "Running {}", query.sql());
    return RowIter<PathAndHash>{
        std::move(query), [](SQLiteQuery& q) -> PathAndHash {
          return {std::string{q.getString(0)}, q.getString(1)};
        }};
  }

  RowIter<SymbolPath> getAllTypePaths(SQLiteTxn& txn) override {
    auto query = txn.query(m_typeStmts.m_getAll);
    XLOGF(DBG9, "Running {}", query.sql());
    return RowIter<SymbolPath>{
        std::move(query), [](SQLiteQuery& q) -> SymbolPath {
          return {q.getString(0), {std::string{q.getString(1)}}};
        }};
  }

  RowIter<SymbolPath> getAllFunctionPaths(SQLiteTxn& txn) override {
    auto query = txn.query(m_functionStmts.m_getAll);
    XLOGF(DBG9, "Running {}", query.sql());
    return RowIter<SymbolPath>{
        std::move(query), [](SQLiteQuery& q) -> SymbolPath {
          return {q.getString(0), {std::string{q.getString(1)}}};
        }};
  }

  RowIter<SymbolPath> getAllConstantPaths(SQLiteTxn& txn) override {
    auto query = txn.query(m_constantStmts.m_getAll);
    XLOGF(DBG9, "Running {}", query.sql());
    return RowIter<SymbolPath>{
        std::move(query), [](SQLiteQuery& q) -> SymbolPath {
          return {q.getString(0), {std::string{q.getString(1)}}};
        }};
  }

  void insertClock(SQLiteTxn& txn, const Clock& clock) override {
    auto query = txn.query(m_clockStmts.m_insert);
    query.bindString("@clock", clock.m_clock);
    query.bindString("@mergebase", clock.m_mergebase);
    XLOGF(DBG9, "Running {}", query.sql());
    query.step();
  }

  Clock getClock(SQLiteTxn& txn) override {
    auto query = txn.query(m_clockStmts.m_get);
    XLOGF(DBG9, "Running {}", query.sql());
    query.step();
    if (!query.row()) {
      return {};
    }
    return {
        .m_clock = std::string{query.getString(0)},
        .m_mergebase = std::string{query.getString(1)}};
  }

  SQLite m_db;
  PathStmts m_pathStmts;
  Sha1HexStmts m_sha1HexStmts;
  TypeStmts m_typeStmts;
  FileStmts m_fileStmts;
  hphp_hash_map<std::tuple<TypeKindMask, DeriveKindMask>, SQLiteStmt>
      m_derivedTypeStmts;
  FunctionStmts m_functionStmts;
  ConstantStmts m_constantStmts;
  ClockStmts m_clockStmts;
};

} // namespace

AutoloadDB::~AutoloadDB() = default;

THREAD_LOCAL(AutoloadDBThreadLocal, t_adb);

AutoloadDB& getDB(const DBData& dbData) {
  AutoloadDBThreadLocal& dbVault = *t_adb.get();
  auto& dbPtr = dbVault[{dbData.m_path.native(), dbData.m_rwMode}];
  if (!dbPtr) {
    dbPtr = std::make_unique<AutoloadDBImpl>(AutoloadDBImpl::get(dbData));
  }
  return *dbPtr;
}

} // namespace Facts
} // namespace HPHP
