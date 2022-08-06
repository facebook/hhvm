// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hh24_types::Checksum;
use hh24_types::DeclHash;
use hh24_types::ToplevelSymbolHash;
use nohash_hasher::IntMap;
use nohash_hasher::IntSet;
use oxidized::file_info::NameType;
use oxidized::relative_path::RelativePath;
use rusqlite::params;
use rusqlite::Connection;
use rusqlite::OptionalExtension;

use crate::datatypes::*;
use crate::FileSummary;

#[derive(Clone, Debug)]
pub struct SymbolItem {
    pub name: String,
    pub kind: NameType,
    pub decl_hash: DeclHash,
}

pub fn create_tables(connection: &mut Connection) -> anyhow::Result<()> {
    let tx = connection.transaction()?;
    tx.prepare_cached(
        "
        CREATE TABLE IF NOT EXISTS NAMING_SYMBOLS (
            HASH INTEGER PRIMARY KEY NOT NULL,
            CANON_HASH INTEGER NOT NULL,
            DECL_HASH INTEGER NOT NULL,
            FLAGS INTEGER NOT NULL,
            FILE_INFO_ID INTEGER NOT NULL
        );",
    )?
    .execute(params![])?;

    tx.prepare_cached(
        "
        CREATE TABLE IF NOT EXISTS NAMING_SYMBOLS_OVERFLOW (
            HASH INTEGER KEY NOT NULL,
            CANON_HASH INTEGER NOT NULL,
            DECL_HASH INTEGER NOT NULL,
            FLAGS INTEGER NOT NULL,
            FILE_INFO_ID INTEGER NOT NULL
        );",
    )?
    .execute(params![])?;

    tx.prepare_cached(
        "
        CREATE TABLE IF NOT EXISTS NAMING_FILE_INFO (
            FILE_INFO_ID INTEGER PRIMARY KEY AUTOINCREMENT,
            PATH_PREFIX_TYPE INTEGER NOT NULL,
            PATH_SUFFIX TEXT NOT NULL,
            TYPE_CHECKER_MODE INTEGER,
            DECL_HASH INTEGER,
            CLASSES TEXT,
            CONSTS TEXT,
            FUNS TEXT,
            TYPEDEFS TEXT
        );",
    )?
    .execute(params![])?;

    tx.prepare_cached("CREATE INDEX IF NOT EXISTS FUNS_CANON ON NAMING_SYMBOLS (CANON_HASH);")?
        .execute(params![])?;

    Ok(tx.commit()?)
}

pub fn derive_checksum(connection: &Connection) -> anyhow::Result<Checksum> {
    let select_statement = "
        SELECT
            NAMING_SYMBOLS.HASH,
            NAMING_SYMBOLS.DECL_HASH
        FROM
            NAMING_SYMBOLS
        ";
    let mut select_statement = connection.prepare_cached(select_statement)?;
    let mut rows = select_statement.query(params![])?;
    let mut checksum = hh24_types::Checksum(0);

    while let Some(row) = rows.next()? {
        checksum.addremove(row.get(0)?, row.get(1)?);
    }

    Ok(checksum)
}

pub fn remove_symbol(
    connection: &Connection,
    symbol_hash: ToplevelSymbolHash,
    path: &RelativePath,
) -> anyhow::Result<()> {
    let file_info_id: FileInfoId = connection
        .prepare(
            "SELECT FILE_INFO_ID FROM NAMING_FILE_INFO
                WHERE PATH_PREFIX_TYPE = ?
                AND PATH_SUFFIX = ?",
        )?
        .query_row(params![path.prefix() as u8, path.path_str()], |row| {
            row.get(0)
        })?;

    connection
        .prepare("DELETE FROM NAMING_SYMBOLS WHERE HASH = ? AND FILE_INFO_ID = ?")?
        .execute(params![symbol_hash, file_info_id])?;
    connection
        .prepare("DELETE FROM NAMING_SYMBOLS_OVERFLOW WHERE HASH = ? AND FILE_INFO_ID = ?")?
        .execute(params![symbol_hash, file_info_id])?;

    if let Some((
        symbol_hash,
        symbol_canon_hash,
        symbol_decl_hash,
        symbol_kind,
        symbol_file_info_id,
        _,
    )) = get_overflow_row(connection, symbol_hash)?
    {
        // Move row from overflow to main symbol table
        connection
            .prepare("DELETE FROM NAMING_SYMBOLS_OVERFLOW WHERE HASH = ? AND FILE_INFO_ID = ?")?
            .execute(params![symbol_hash, symbol_file_info_id])?;
        let insert_statement = "
        INSERT INTO NAMING_SYMBOLS (
            HASH,
            CANON_HASH,
            DECL_HASH,
            FLAGS,
            FILE_INFO_ID
        ) VALUES (
            ?, ?, ?, ?, ?
        );";
        connection.prepare(insert_statement)?.execute(params![
            symbol_hash,
            symbol_canon_hash,
            symbol_decl_hash,
            symbol_kind,
            symbol_file_info_id
        ])?;
    }

    Ok(())
}

pub fn insert_file_summary(
    connection: &Connection,
    path: &RelativePath,
    file_info_id: i64,
    dep_type: typing_deps_hash::DepType,
    items: impl Iterator<Item = SymbolItem>,
) -> anyhow::Result<()> {
    let insert_statement = "
        INSERT INTO NAMING_SYMBOLS (
            HASH,
            CANON_HASH,
            DECL_HASH,
            FLAGS,
            FILE_INFO_ID
        ) VALUES (
            ?, ?, ?, ?, ?
        );";
    let insert_overflow_statement = "
        INSERT INTO NAMING_SYMBOLS_OVERFLOW (
            HASH,
            CANON_HASH,
            DECL_HASH,
            FLAGS,
            FILE_INFO_ID
        ) VALUES (
            ?, ?, ?, ?, ?
        );";

    let mut insert_statement = connection.prepare(insert_statement)?;
    let mut insert_overflow_statement = connection.prepare(insert_overflow_statement)?;

    for mut item in items {
        let decl_hash = item.decl_hash;
        let hash = convert::name_to_hash(dep_type, &item.name);
        item.name.make_ascii_lowercase();
        let canon_hash = convert::name_to_hash(dep_type, &item.name);
        let kind = item.kind as i64;

        if let Some((
            symbol_hash,
            symbol_canon_hash,
            symbol_decl_hash,
            symbol_kind,
            symbol_file_info_id,
            symbol_path,
        )) = get_row(connection, ToplevelSymbolHash::from_u64(hash as u64))?
        {
            // check if new entry appears first alphabetically
            if path.path_str() < symbol_path.path_str() {
                // delete symbol from symbols
                connection
                    .prepare("DELETE FROM NAMING_SYMBOLS WHERE HASH = ? AND FILE_INFO_ID = ?")?
                    .execute(params![symbol_hash, symbol_file_info_id])?;

                // insert old row into overflow
                insert_overflow_statement.execute(params![
                    symbol_hash,
                    symbol_canon_hash,
                    symbol_decl_hash,
                    symbol_kind,
                    symbol_file_info_id
                ])?;

                // insert new row into main table
                insert_statement.execute(params![
                    hash,
                    canon_hash,
                    decl_hash,
                    kind,
                    file_info_id
                ])?;
            } else {
                // insert new row into overflow table
                insert_overflow_statement.execute(params![
                    hash,
                    canon_hash,
                    decl_hash,
                    kind,
                    file_info_id
                ])?;
            }
        } else {
            // No collision. Insert as you normally would
            insert_statement.execute(params![hash, canon_hash, decl_hash, kind, file_info_id])?;
        }
    }
    Ok(())
}

fn get_overflow_row(
    connection: &Connection,
    symbol_hash: ToplevelSymbolHash,
) -> anyhow::Result<
    Option<(
        ToplevelSymbolHash,
        ToplevelCanonSymbolHash,
        DeclHash,
        ToplevelSymbolFlags,
        FileInfoId,
        RelativePath,
    )>,
> {
    let select_statement = "
        SELECT
            NAMING_SYMBOLS_OVERFLOW.HASH,
            NAMING_SYMBOLS_OVERFLOW.CANON_HASH,
            NAMING_SYMBOLS_OVERFLOW.DECL_HASH,
            NAMING_SYMBOLS_OVERFLOW.FLAGS,
            NAMING_SYMBOLS_OVERFLOW.FILE_INFO_ID,
            NAMING_FILE_INFO.PATH_PREFIX_TYPE,
            NAMING_FILE_INFO.PATH_SUFFIX
        FROM
            NAMING_SYMBOLS_OVERFLOW
        LEFT JOIN
            NAMING_FILE_INFO
        ON
            NAMING_SYMBOLS_OVERFLOW.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE
            NAMING_SYMBOLS_OVERFLOW.HASH = ?
        ORDER BY
            NAMING_FILE_INFO.PATH_SUFFIX ASC
        ";

    let mut select_statement = connection.prepare_cached(select_statement)?;
    let result = select_statement
        .query_row(params![symbol_hash], |row| {
            let prefix: SqlitePrefix = row.get(5)?;
            let suffix: SqlitePathBuf = row.get(6)?;
            let path = RelativePath::make(prefix.value, suffix.value);
            Ok((
                row.get(0)?,
                row.get(1)?,
                row.get(2)?,
                row.get(3)?,
                row.get(4)?,
                path,
            ))
        })
        .optional();

    Ok(result?)
}

fn get_row(
    connection: &Connection,
    symbol_hash: ToplevelSymbolHash,
) -> anyhow::Result<
    Option<(
        ToplevelSymbolHash,
        ToplevelCanonSymbolHash,
        DeclHash,
        ToplevelSymbolFlags,
        FileInfoId,
        RelativePath,
    )>,
> {
    let select_statement = "
        SELECT
            NAMING_SYMBOLS.HASH,
            NAMING_SYMBOLS.CANON_HASH,
            NAMING_SYMBOLS.DECL_HASH,
            NAMING_SYMBOLS.FLAGS,
            NAMING_SYMBOLS.FILE_INFO_ID,
            NAMING_FILE_INFO.PATH_PREFIX_TYPE,
            NAMING_FILE_INFO.PATH_SUFFIX
        FROM
            NAMING_SYMBOLS
        LEFT JOIN
            NAMING_FILE_INFO
        ON
            NAMING_SYMBOLS.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE
            NAMING_SYMBOLS.HASH = ?
        ";

    let mut select_statement = connection.prepare_cached(select_statement)?;
    let result = select_statement
        .query_row(params![symbol_hash], |row| {
            let prefix: SqlitePrefix = row.get(5)?;
            let suffix: SqlitePathBuf = row.get(6)?;
            let path = RelativePath::make(prefix.value, suffix.value);
            Ok((
                row.get(0)?,
                row.get(1)?,
                row.get(2)?,
                row.get(3)?,
                row.get(4)?,
                path,
            ))
        })
        .optional();

    Ok(result?)
}

pub fn get_path(
    connection: &Connection,
    symbol_hash: ToplevelSymbolHash,
) -> anyhow::Result<Option<(RelativePath, NameType)>> {
    let select_statement = "
        SELECT
            NAMING_FILE_INFO.PATH_PREFIX_TYPE,
            NAMING_FILE_INFO.PATH_SUFFIX,
            NAMING_SYMBOLS.FLAGS
        FROM
            NAMING_SYMBOLS
        LEFT JOIN
            NAMING_FILE_INFO
        ON
            NAMING_SYMBOLS.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE
            NAMING_SYMBOLS.HASH = ?
        LIMIT 1
        ";

    let mut select_statement = connection.prepare_cached(select_statement)?;
    let result = select_statement
        .query_row(params![symbol_hash], |row| {
            let prefix: SqlitePrefix = row.get(0)?;
            let suffix: SqlitePathBuf = row.get(1)?;
            let kind: NameType = row.get(2)?;
            Ok((RelativePath::make(prefix.value, suffix.value), kind))
        })
        .optional();

    Ok(result?)
}

pub fn get_path_case_insensitive(
    connection: &Connection,
    symbol_hash: ToplevelCanonSymbolHash,
) -> anyhow::Result<Option<RelativePath>> {
    let select_statement = "
        SELECT
            NAMING_FILE_INFO.PATH_PREFIX_TYPE,
            NAMING_FILE_INFO.PATH_SUFFIX
        FROM
            NAMING_SYMBOLS
        LEFT JOIN
            NAMING_FILE_INFO
        ON
            NAMING_SYMBOLS.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE
        NAMING_SYMBOLS.CANON_HASH = ?
        ";

    let mut select_statement = connection.prepare_cached(select_statement)?;
    let result = select_statement
        .query_row(params![symbol_hash], |row| {
            let prefix: SqlitePrefix = row.get(0)?;
            let suffix: SqlitePathBuf = row.get(1)?;
            Ok(RelativePath::make(prefix.value, suffix.value))
        })
        .optional();

    Ok(result?)
}

pub fn get_type_name_case_insensitive(
    connection: &Connection,
    symbol_hash: ToplevelCanonSymbolHash,
) -> anyhow::Result<Option<String>> {
    let select_statement = "
        SELECT
            NAMING_FILE_INFO.CLASSES,
            NAMING_FILE_INFO.TYPEDEFS
        FROM
            NAMING_SYMBOLS
        LEFT JOIN
            NAMING_FILE_INFO
        ON
            NAMING_SYMBOLS.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE
            NAMING_SYMBOLS.CANON_HASH = ?
        LIMIT 1
        ";

    let mut select_statement = connection.prepare_cached(select_statement)?;
    let names_opt = select_statement
        .query_row(params![symbol_hash], |row| {
            let classes: String = row.get(0)?;
            let typedefs: String = row.get(1)?;
            Ok((classes, typedefs))
        })
        .optional()?;

    if let Some((classes, typedefs)) = names_opt {
        for class in classes.split('|') {
            if symbol_hash == ToplevelCanonSymbolHash::from_type(class.to_owned()) {
                return Ok(Some(class.to_owned()));
            }
        }
        for typedef in typedefs.split('|') {
            if symbol_hash == ToplevelCanonSymbolHash::from_type(typedef.to_owned()) {
                return Ok(Some(typedef.to_owned()));
            }
        }
    }
    Ok(None)
}

pub fn get_fun_name_case_insensitive(
    connection: &Connection,
    symbol_hash: ToplevelCanonSymbolHash,
) -> anyhow::Result<Option<String>> {
    let select_statement = "
        SELECT
            NAMING_FILE_INFO.FUNS
        FROM
            NAMING_SYMBOLS
        LEFT JOIN
            NAMING_FILE_INFO
        ON
            NAMING_SYMBOLS.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE
            NAMING_SYMBOLS.CANON_HASH = ?
        LIMIT 1
        ";

    let mut select_statement = connection.prepare_cached(select_statement)?;
    let names_opt = select_statement
        .query_row(params![symbol_hash], |row| {
            let funs: String = row.get(0)?;
            Ok(funs)
        })
        .optional()?;

    if let Some(funs) = names_opt {
        for fun in funs.split('|') {
            if symbol_hash == ToplevelCanonSymbolHash::from_fun(fun.to_owned()) {
                return Ok(Some(fun.to_owned()));
            }
        }
    }
    Ok(None)
}

pub fn get_symbol_hashes(
    connection: &Connection,
    path: &RelativePath,
) -> anyhow::Result<IntSet<ToplevelSymbolHash>> {
    let select_statement = "
        SELECT
            NAMING_SYMBOLS.HASH
        FROM
            NAMING_SYMBOLS
        LEFT JOIN
            NAMING_FILE_INFO
        ON
            NAMING_SYMBOLS.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE
            NAMING_FILE_INFO.PATH_PREFIX_TYPE = ? AND
            NAMING_FILE_INFO.PATH_SUFFIX = ?
        ";
    let mut select_statement = connection.prepare_cached(select_statement)?;
    let mut rows = select_statement.query(params![path.prefix() as u8, path.path_str()])?;
    let mut symbol_hashes = IntSet::default();

    while let Some(row) = rows.next()? {
        symbol_hashes.insert(row.get(0)?);
    }

    Ok(symbol_hashes)
}

pub fn get_overflow_symbol_hashes(
    connection: &Connection,
    path: &RelativePath,
) -> anyhow::Result<IntSet<ToplevelSymbolHash>> {
    let select_statement = "
        SELECT
            NAMING_SYMBOLS_OVERFLOW.HASH
        FROM
            NAMING_SYMBOLS_OVERFLOW
        LEFT JOIN
            NAMING_FILE_INFO
        ON
            NAMING_SYMBOLS_OVERFLOW.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE
            NAMING_FILE_INFO.PATH_PREFIX_TYPE = ? AND
            NAMING_FILE_INFO.PATH_SUFFIX = ?
        ";
    let mut select_statement = connection.prepare_cached(select_statement)?;
    let mut rows = select_statement.query(params![path.prefix() as u8, path.path_str()])?;
    let mut symbol_hashes = IntSet::default();

    while let Some(row) = rows.next()? {
        symbol_hashes.insert(row.get(0)?);
    }

    Ok(symbol_hashes)
}

pub fn get_symbol_and_decl_hashes(
    connection: &Connection,
    path: &RelativePath,
) -> anyhow::Result<IntMap<ToplevelSymbolHash, DeclHash>> {
    let select_statement = "
        SELECT
            NAMING_SYMBOLS.HASH,
            NAMING_SYMBOLS.DECL_HASH
        FROM
            NAMING_SYMBOLS
        LEFT JOIN
            NAMING_FILE_INFO
        ON
            NAMING_SYMBOLS.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE
            NAMING_FILE_INFO.PATH_PREFIX_TYPE = ? AND
            NAMING_FILE_INFO.PATH_SUFFIX = ?
        ";
    let mut select_statement = connection.prepare_cached(select_statement)?;
    let mut rows = select_statement.query(params![path.prefix() as u8, path.path_str()])?;
    let mut symbol_and_decl_hashes = IntMap::default();
    while let Some(row) = rows.next()? {
        symbol_and_decl_hashes.insert(row.get(0)?, row.get(1)?);
    }

    Ok(symbol_and_decl_hashes)
}

pub fn insert_file_info(
    connection: &Connection,
    path_rel: &RelativePath,
    file_summary: &FileSummary,
) -> anyhow::Result<()> {
    let prefix_type = path_rel.prefix() as u8; // TODO(ljw): shouldn't this use prefix_to_i64?
    let suffix = path_rel.path().to_str().unwrap();
    let type_checker_mode = convert::mode_to_i64(file_summary.mode);
    let hash = file_summary.hash;

    connection
        .prepare_cached(
            "INSERT INTO NAMING_FILE_INFO(
                PATH_PREFIX_TYPE,
                PATH_SUFFIX,
                TYPE_CHECKER_MODE,
                DECL_HASH,
                CLASSES,
                CONSTS,
                FUNS,
                TYPEDEFS
            )
            VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8);",
        )?
        .execute(params![
            prefix_type,
            suffix,
            type_checker_mode,
            hash,
            join_with_pipe(file_summary.classes()),
            join_with_pipe(file_summary.consts()),
            join_with_pipe(file_summary.funs()),
            join_with_pipe(file_summary.typedefs()),
        ])?;
    Ok(())
}

fn join_with_pipe<'a>(symbols: impl Iterator<Item = (&'a str, DeclHash)>) -> String {
    let mut s = String::new();
    for (symbol, _) in symbols {
        s.push_str(symbol);
        s.push('|');
    }
    s.pop(); // Remove trailing pipe character
    s
}

pub fn delete(connection: &Connection, path: &RelativePath) -> anyhow::Result<()> {
    let file_info_id: Option<FileInfoId> = connection
        .prepare_cached(
            "SELECT FILE_INFO_ID FROM NAMING_FILE_INFO
                WHERE PATH_PREFIX_TYPE = ?
                AND PATH_SUFFIX = ?",
        )?
        .query_row(params![path.prefix() as u8, path.path_str()], |row| {
            row.get(0)
        })
        .optional()?;
    let file_info_id = match file_info_id {
        Some(id) => id,
        None => return Ok(()),
    };
    connection
        .prepare_cached("DELETE FROM NAMING_FILE_INFO WHERE FILE_INFO_ID = ?")?
        .execute(params![file_info_id])?;
    Ok(())
}

pub fn get_decl_hash(
    connection: &Connection,
    symbol_hash: ToplevelSymbolHash,
) -> anyhow::Result<Option<DeclHash>> {
    let result = connection
        .prepare_cached("SELECT DECL_HASH FROM NAMING_SYMBOLS WHERE HASH = ?")?
        .query_row(params![symbol_hash], |row| row.get(0))
        .optional();
    Ok(result?)
}
