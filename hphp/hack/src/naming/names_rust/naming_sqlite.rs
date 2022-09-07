// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

use hh24_types::Checksum;
use hh24_types::DeclHash;
use hh24_types::ToplevelCanonSymbolHash;
use hh24_types::ToplevelSymbolHash;
use nohash_hasher::IntSet;
use oxidized::file_info::NameType;
use oxidized::relative_path::RelativePath;
use rusqlite::params;
use rusqlite::Connection;
use rusqlite::OptionalExtension;

pub struct Names {
    conn: rusqlite::Connection,
}

impl Names {
    pub fn from_file(path: impl AsRef<Path>) -> anyhow::Result<Self> {
        let path = path.as_ref();
        let mut conn = Connection::open(path)?;
        Self::create_tables(&mut conn)?;
        Ok(Self { conn })
    }

    pub fn new_in_memory() -> anyhow::Result<Self> {
        let mut conn = Connection::open_in_memory()?;
        Self::create_tables(&mut conn)?;
        Ok(Self { conn })
    }

    pub fn from_connection(mut conn: Connection) -> anyhow::Result<Self> {
        Self::create_tables(&mut conn)?;
        Ok(Self { conn })
    }

    pub fn backup(&self, path: &Path) -> anyhow::Result<()> {
        self.conn.backup(rusqlite::DatabaseName::Main, path, None)?;
        Ok(())
    }

    fn create_tables(conn: &mut Connection) -> anyhow::Result<()> {
        let tx = conn.transaction()?;
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

        // TODO(ljw) - NAMING_FILE_INFO should be indexed on PATH_SUFFIX.

        tx.prepare_cached("CREATE INDEX IF NOT EXISTS FUNS_CANON ON NAMING_SYMBOLS (CANON_HASH);")?
            .execute(params![])?;

        Ok(tx.commit()?)
    }

    /// Walks O(table) to derive checksum based on the decl_hash of each item.
    /// TODO(ljw) - We should store checksum in the table directly.
    pub fn derive_checksum(&self) -> anyhow::Result<Checksum> {
        let select_statement = "
        SELECT
            NAMING_SYMBOLS.HASH,
            NAMING_SYMBOLS.DECL_HASH
        FROM
            NAMING_SYMBOLS
        ";
        let mut select_statement = self.conn.prepare_cached(select_statement)?;
        let mut rows = select_statement.query(params![])?;
        let mut checksum = hh24_types::Checksum(0);

        while let Some(row) = rows.next()? {
            checksum.addremove(row.get(0)?, row.get(1)?);
        }

        Ok(checksum)
    }

    /// Removes a symbol definition from the reverse naming table (be it in
    /// a winner or in the overflow table), and fixes the remaining candidates.
    /// TODO(ljw): this crashes in the case where you delete an overflow symbol
    /// and it tries to move the remaining overflow symbol into the main table.
    pub fn remove_symbol(
        &self,
        symbol_hash: ToplevelSymbolHash,
        path: &RelativePath,
    ) -> anyhow::Result<()> {
        let file_info_id: crate::FileInfoId = self
            .conn
            .prepare(
                "SELECT FILE_INFO_ID FROM NAMING_FILE_INFO
                WHERE PATH_PREFIX_TYPE = ?
                AND PATH_SUFFIX = ?",
            )?
            .query_row(params![path.prefix() as u8, path.path_str()], |row| {
                row.get(0)
            })?;

        self.conn
            .prepare("DELETE FROM NAMING_SYMBOLS WHERE HASH = ? AND FILE_INFO_ID = ?")?
            .execute(params![symbol_hash, file_info_id])?;
        self.conn
            .prepare("DELETE FROM NAMING_SYMBOLS_OVERFLOW WHERE HASH = ? AND FILE_INFO_ID = ?")?
            .execute(params![symbol_hash, file_info_id])?;

        let mut overflow_symbols = self.get_overflow_rows_unordered(symbol_hash)?;
        overflow_symbols.sort_by_key(|symbol| symbol.path.clone());
        if let Some(symbol) = overflow_symbols.into_iter().next() {
            // Move row from overflow to main symbol table
            self.conn
                .prepare("DELETE FROM NAMING_SYMBOLS_OVERFLOW WHERE HASH = ? AND FILE_INFO_ID = ?")?
                .execute(params![symbol_hash, symbol.file_info_id])?;
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
            self.conn.prepare(insert_statement)?.execute(params![
                symbol.hash,
                symbol.canon_hash,
                symbol.decl_hash,
                symbol.kind,
                symbol.file_info_id
            ])?;
        }

        Ok(())
    }

    /// Adds all of a file's symbols into the reverse naming table,
    /// into normal or overflow as appropriate.
    pub fn save_file_summary(
        &self,
        path: &RelativePath,
        file_summary: &crate::FileSummary,
    ) -> anyhow::Result<()> {
        let file_info_id = self.insert_file_info_and_get_file_id(path, file_summary)?;
        self.insert_file_summary(
            path,
            file_info_id,
            typing_deps_hash::DepType::Fun,
            file_summary
                .funs()
                .map(|(name, decl_hash)| crate::DeclSummary {
                    symbol: name.to_owned(),
                    name_type: NameType::Fun,
                    hash: decl_hash,
                }),
        )?;
        self.insert_file_summary(
            path,
            file_info_id,
            typing_deps_hash::DepType::GConst,
            file_summary
                .consts()
                .map(|(name, decl_hash)| crate::DeclSummary {
                    symbol: name.to_owned(),
                    name_type: NameType::Const,
                    hash: decl_hash,
                }),
        )?;
        self.insert_file_summary(
            path,
            file_info_id,
            typing_deps_hash::DepType::Type,
            file_summary
                .classes()
                .map(|(name, decl_hash)| crate::DeclSummary {
                    symbol: name.to_owned(),
                    name_type: NameType::Class,
                    hash: decl_hash,
                }),
        )?;
        self.insert_file_summary(
            path,
            file_info_id,
            typing_deps_hash::DepType::Type,
            file_summary
                .typedefs()
                .map(|(name, decl_hash)| crate::DeclSummary {
                    symbol: name.to_owned(),
                    name_type: NameType::Typedef,
                    hash: decl_hash,
                }),
        )?;
        Ok(())
    }

    /// private helper for saved_file_summary
    fn insert_file_summary(
        &self,
        path: &RelativePath,
        file_info_id: crate::FileInfoId,
        dep_type: typing_deps_hash::DepType,
        items: impl Iterator<Item = crate::DeclSummary>,
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

        let mut insert_statement = self.conn.prepare(insert_statement)?;
        let mut insert_overflow_statement = self.conn.prepare(insert_overflow_statement)?;

        for mut item in items {
            let decl_hash = item.hash;
            let hash = crate::datatypes::convert::name_to_hash(dep_type, &item.symbol);
            item.symbol.make_ascii_lowercase();
            let canon_hash = crate::datatypes::convert::name_to_hash(dep_type, &item.symbol);
            let kind = item.name_type;

            if let Some(symbol) = self.get_row(ToplevelSymbolHash::from_u64(hash as u64))? {
                // check if new entry appears first alphabetically
                if path.path_str() < symbol.path.path_str() {
                    // delete symbol from symbols
                    self.conn
                        .prepare("DELETE FROM NAMING_SYMBOLS WHERE HASH = ? AND FILE_INFO_ID = ?")?
                        .execute(params![symbol.hash, symbol.file_info_id])?;

                    // insert old row into overflow
                    insert_overflow_statement.execute(params![
                        symbol.hash,
                        symbol.canon_hash,
                        symbol.decl_hash,
                        symbol.kind,
                        symbol.file_info_id
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
                insert_statement.execute(params![
                    hash,
                    canon_hash,
                    decl_hash,
                    kind,
                    file_info_id
                ])?;
            }
        }
        Ok(())
    }

    /// Gets all overflow rows in the reverse naming table for a given symbol hash,
    /// and joins with the forward naming table to resolve filenames.
    pub fn get_overflow_rows_unordered(
        &self,
        symbol_hash: ToplevelSymbolHash,
    ) -> anyhow::Result<Vec<crate::SymbolRow>> {
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
        ";

        let mut select_statement = self.conn.prepare_cached(select_statement)?;
        let mut rows = select_statement.query(params![symbol_hash])?;
        let mut result = vec![];
        while let Some(row) = rows.next()? {
            let prefix: crate::datatypes::SqlitePrefix = row.get(5)?;
            let suffix: crate::datatypes::SqlitePathBuf = row.get(6)?;
            let path = RelativePath::make(prefix.value, suffix.value);
            result.push(crate::SymbolRow {
                hash: row.get(0)?,
                canon_hash: row.get(1)?,
                decl_hash: row.get(2)?,
                kind: row.get(3)?,
                file_info_id: row.get(4)?,
                path,
            });
        }
        Ok(result)
    }

    /// Gets the winning entry for a symbol from the reverse naming table,
    /// and joins with forward-naming-table to get filename.
    pub fn get_row(
        &self,
        symbol_hash: ToplevelSymbolHash,
    ) -> anyhow::Result<Option<crate::SymbolRow>> {
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

        let mut select_statement = self.conn.prepare_cached(select_statement)?;
        let result = select_statement
            .query_row(params![symbol_hash], |row| {
                let prefix: crate::datatypes::SqlitePrefix = row.get(5)?;
                let suffix: crate::datatypes::SqlitePathBuf = row.get(6)?;
                let path = RelativePath::make(prefix.value, suffix.value);
                Ok(crate::SymbolRow {
                    hash: row.get(0)?,
                    canon_hash: row.get(1)?,
                    decl_hash: row.get(2)?,
                    kind: row.get(3)?,
                    file_info_id: row.get(4)?,
                    path,
                })
            })
            .optional();

        Ok(result?)
    }

    /// This looks up the reverse naming table by hash, to fetch the decl-hash
    pub fn get_decl_hash(
        &self,
        symbol_hash: ToplevelSymbolHash,
    ) -> anyhow::Result<Option<DeclHash>> {
        let result = self
            .conn
            .prepare_cached("SELECT DECL_HASH FROM NAMING_SYMBOLS WHERE HASH = ?")?
            .query_row(params![symbol_hash], |row| row.get(0))
            .optional();
        Ok(result?)
    }

    /// Looks up reverse-naming-table winner by symbol hash.
    /// Similar to get_path_by_symbol_hash, but includes the name kind.
    pub fn get_filename(
        &self,
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

        let mut select_statement = self.conn.prepare_cached(select_statement)?;
        let result = select_statement
            .query_row(params![symbol_hash], |row| {
                let prefix: crate::datatypes::SqlitePrefix = row.get(0)?;
                let suffix: crate::datatypes::SqlitePathBuf = row.get(1)?;
                let kind: NameType = row.get(2)?;
                Ok((RelativePath::make(prefix.value, suffix.value), kind))
            })
            .optional();

        Ok(result?)
    }

    /// Looks up reverse-naming-table winner by symbol hash.
    /// Similar to get_filename, but discards the name kind
    pub fn get_path_by_symbol_hash(
        &self,
        symbol_hash: ToplevelSymbolHash,
    ) -> anyhow::Result<Option<RelativePath>> {
        match self.get_filename(symbol_hash)? {
            Some((path, _kind)) => Ok(Some(path)),
            None => Ok(None),
        }
    }

    /// Looks up reverse-naming-table winner by case-insensitive symbol hash.
    pub fn get_path_case_insensitive(
        &self,
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

        let mut select_statement = self.conn.prepare_cached(select_statement)?;
        let result = select_statement
            .query_row(params![symbol_hash], |row| {
                let prefix: crate::datatypes::SqlitePrefix = row.get(0)?;
                let suffix: crate::datatypes::SqlitePathBuf = row.get(1)?;
                Ok(RelativePath::make(prefix.value, suffix.value))
            })
            .optional();

        Ok(result?)
    }

    /// This function shouldn't really exist.
    /// It searches the reverse-naming-table by case-insensitive hash.
    /// Then looks up the forward-naming-table entry for that winner.
    /// Then it iterates the string type names stored in that forward-naming-table entry,
    /// comparing them one by one until it finds one whose case-insensitive hash
    /// matches what was asked for.
    pub fn get_type_name_case_insensitive(
        &self,
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

        let mut select_statement = self.conn.prepare_cached(select_statement)?;
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

    /// This function shouldn't really exist.
    /// It searches the reverse-naming-table by case-insensitive hash.
    /// Then looks up the forward-naming-table entry for that winner.
    /// Then it iterates the string fun names stored in that forward-naming-table entry,
    /// comparing them one by one until it finds one whose case-insensitive hash
    /// matches what was asked for.
    pub fn get_fun_name_case_insensitive(
        &self,
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

        let mut select_statement = self.conn.prepare_cached(select_statement)?;
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

    /// It scans O(table) for every entry in the reverse naming table that matches
    /// the filename, and returns them.
    /// TODO(ljw) This function does a needless O(table) scan.
    /// It should get the same information more cheaply by lookup up the forward
    /// naming table directly.
    pub fn get_symbol_hashes(
        &self,
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
        let mut select_statement = self.conn.prepare_cached(select_statement)?;
        let mut rows = select_statement.query(params![path.prefix() as u8, path.path_str()])?;
        let mut symbol_hashes = IntSet::default();

        while let Some(row) = rows.next()? {
            symbol_hashes.insert(row.get(0)?);
        }

        Ok(symbol_hashes)
    }

    /// It scans O(table) for every entry in the overflow reverse naming table that matches
    /// the filename, and returns them.
    /// TODO(ljw) This function does a needless O(table) scan.
    /// It should get the same information more cheaply by lookup up the forward
    /// naming table directly.
    pub fn get_overflow_symbol_hashes(
        &self,
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
        let mut select_statement = self.conn.prepare_cached(select_statement)?;
        let mut rows = select_statement.query(params![path.prefix() as u8, path.path_str()])?;
        let mut symbol_hashes = IntSet::default();

        while let Some(row) = rows.next()? {
            symbol_hashes.insert(row.get(0)?);
        }

        Ok(symbol_hashes)
    }

    /// It scans O(table) for every entry in the reverse naming table that matches
    /// the filename, and returns them.
    /// TODO(ljw) This function does a needless O(table) scan.
    /// It should get the same information more cheaply by lookup up the forward
    /// naming table directly.
    pub fn get_symbol_and_decl_hashes(
        &self,
        path: &RelativePath,
    ) -> anyhow::Result<Vec<(ToplevelSymbolHash, DeclHash, crate::FileInfoId)>> {
        let select_statement = "
        SELECT
            NAMING_SYMBOLS.HASH,
            NAMING_SYMBOLS.DECL_HASH,
            NAMING_SYMBOLS.FILE_INFO_ID
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
        let mut select_statement = self.conn.prepare_cached(select_statement)?;
        let mut rows = select_statement.query(params![path.prefix() as u8, path.path_str()])?;
        let mut result = vec![];
        while let Some(row) = rows.next()? {
            result.push((row.get(0)?, row.get(1)?, row.get(2)?));
        }
        Ok(result)
    }

    /// This inserts an item into the forward naming table.
    fn insert_file_info_and_get_file_id(
        &self,
        path_rel: &RelativePath,
        file_summary: &crate::FileSummary,
    ) -> anyhow::Result<crate::FileInfoId> {
        let prefix_type = path_rel.prefix() as u8; // TODO(ljw): shouldn't this use prefix_to_i64?
        let suffix = path_rel.path().to_str().unwrap();
        let type_checker_mode = crate::datatypes::convert::mode_to_i64(file_summary.mode);
        let hash = file_summary.hash;

        self.conn
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
                Self::join_with_pipe(file_summary.classes()),
                Self::join_with_pipe(file_summary.consts()),
                Self::join_with_pipe(file_summary.funs()),
                Self::join_with_pipe(file_summary.typedefs()),
            ])?;
        let file_info_id = crate::FileInfoId::last_insert_rowid(&self.conn);
        Ok(file_info_id)
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

    /// This removes an entry from the forward naming table.
    pub fn delete(&self, path: &RelativePath) -> anyhow::Result<()> {
        let file_info_id: Option<crate::FileInfoId> = self
            .conn
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
        self.conn
            .prepare_cached("DELETE FROM NAMING_FILE_INFO WHERE FILE_INFO_ID = ?")?
            .execute(params![file_info_id])?;
        Ok(())
    }
}
