// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

use anyhow::Context;
use hh24_types::Checksum;
use hh24_types::DeclHash;
use hh24_types::ToplevelCanonSymbolHash;
use hh24_types::ToplevelSymbolHash;
use nohash_hasher::IntSet;
use oxidized::file_info::NameType;
use relative_path::RelativePath;
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
        Self::create_indices(&mut conn)?;
        Ok(Self { conn })
    }

    pub fn new_in_memory() -> anyhow::Result<Self> {
        let mut conn = Connection::open_in_memory()?;
        Self::create_tables(&mut conn)?;
        Self::create_indices(&mut conn)?;
        Ok(Self { conn })
    }

    pub fn from_connection(mut conn: Connection) -> anyhow::Result<Self> {
        Self::create_tables(&mut conn)?;
        Self::create_indices(&mut conn)?;
        Ok(Self { conn })
    }

    pub fn backup(&self, path: &Path) -> anyhow::Result<()> {
        self.conn.backup(rusqlite::DatabaseName::Main, path, None)?;
        Ok(())
    }

    fn create_tables(conn: &mut Connection) -> anyhow::Result<()> {
        conn.execute(
            "
            CREATE TABLE IF NOT EXISTS NAMING_SYMBOLS (
                HASH INTEGER PRIMARY KEY NOT NULL,
                CANON_HASH INTEGER NOT NULL,
                DECL_HASH INTEGER NOT NULL,
                FLAGS INTEGER NOT NULL,
                FILE_INFO_ID INTEGER NOT NULL
            );",
            params![],
        )?;

        conn.execute(
            "
            CREATE TABLE IF NOT EXISTS NAMING_SYMBOLS_OVERFLOW (
                HASH INTEGER KEY NOT NULL,
                CANON_HASH INTEGER NOT NULL,
                DECL_HASH INTEGER NOT NULL,
                FLAGS INTEGER NOT NULL,
                FILE_INFO_ID INTEGER NOT NULL
            );",
            params![],
        )?;

        conn.execute(
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
                TYPEDEFS TEXT,
                MODULES TEXT
            );",
            params![],
        )?;

        conn.execute(
            "
            CREATE TABLE IF NOT EXISTS CHECKSUM (
                ID INTEGER PRIMARY KEY,
                CHECKSUM_VALUE INTEGER NOT NULL
            );
            ",
            params![],
        )?;

        conn.execute(
            "INSERT OR IGNORE INTO CHECKSUM (ID, CHECKSUM_VALUE) VALUES (0, 0);",
            params![],
        )?;

        Ok(())
    }

    pub fn create_indices(conn: &mut Connection) -> anyhow::Result<()> {
        conn.execute(
            "CREATE UNIQUE INDEX IF NOT EXISTS FILE_INFO_PATH_IDX
             ON NAMING_FILE_INFO (PATH_SUFFIX, PATH_PREFIX_TYPE);",
            params![],
        )?;

        conn.execute(
            "CREATE INDEX IF NOT EXISTS TYPES_CANON
             ON NAMING_SYMBOLS (CANON_HASH);",
            params![],
        )?;

        Ok(())
    }

    pub fn get_checksum(&self) -> anyhow::Result<Checksum> {
        Ok(self
            .conn
            .prepare_cached("SELECT CHECKSUM_VALUE FROM CHECKSUM")?
            .query_row(params![], |row| row.get(0))?)
    }

    pub fn set_checksum(&self, checksum: Checksum) -> anyhow::Result<()> {
        self.conn
            .prepare_cached("REPLACE INTO CHECKSUM (ID, CHECKSUM_VALUE) VALUES (0, ?);")?
            .execute(params![checksum])?;
        Ok(())
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

    /// Adds all of a file's symbols into the forward and reverse naming tables,
    /// into normal or overflow reverse table as appropriate.
    pub(crate) fn save_file_summary(
        &self,
        path: &RelativePath,
        summary: &crate::FileSummary,
    ) -> anyhow::Result<()> {
        let mut save_result = crate::SaveResult {
            checksum: self.get_checksum()?,
            ..Default::default()
        };
        self.insert_file_summary(path, summary, &mut save_result)?;
        self.set_checksum(save_result.checksum)?;
        Ok(())
    }

    // private helper for `save_file_summary`/`build`
    fn insert_file_summary(
        &self,
        path: &RelativePath,
        summary: &crate::FileSummary,
        save_result: &mut crate::SaveResult,
    ) -> anyhow::Result<()> {
        let file_info_id = self.insert_file_info_and_get_file_id(path, summary)?;
        save_result.files_added += 1;
        self.insert_symbols(path, file_info_id, summary.funs(), save_result)?;
        self.insert_symbols(path, file_info_id, summary.consts(), save_result)?;
        self.insert_symbols(path, file_info_id, summary.classes(), save_result)?;
        self.insert_symbols(path, file_info_id, summary.typedefs(), save_result)?;
        self.insert_symbols(path, file_info_id, summary.modules(), save_result)?;
        Ok(())
    }

    // private helper for insert_file_summary
    fn insert_symbols<'a>(
        &self,
        path: &RelativePath,
        file_id: crate::FileInfoId,
        items: impl Iterator<Item = &'a crate::DeclSummary>,
        save_result: &mut crate::SaveResult,
    ) -> anyhow::Result<()> {
        for item in items {
            self.try_insert_symbol(path, file_id, item.clone(), save_result)
                .with_context(|| {
                    format!(
                        "Failed to insert {:?} {} (defined in {path})",
                        item.name_type, item.symbol
                    )
                })?
        }
        Ok(())
    }

    // private helper for insert_symbols
    fn try_insert_symbol(
        &self,
        path: &RelativePath,
        file_info_id: crate::FileInfoId,
        item: crate::DeclSummary,
        save_result: &mut crate::SaveResult,
    ) -> anyhow::Result<()> {
        let mut insert_statement = self.conn.prepare_cached(
            "INSERT INTO NAMING_SYMBOLS (HASH, CANON_HASH, DECL_HASH, FLAGS, FILE_INFO_ID)
            VALUES (?, ?, ?, ?, ?);",
        )?;
        let mut insert_overflow_statement = self.conn.prepare_cached(
            "INSERT INTO NAMING_SYMBOLS_OVERFLOW (HASH, CANON_HASH, DECL_HASH, FLAGS, FILE_INFO_ID)
            VALUES (?, ?, ?, ?, ?);",
        )?;
        let mut delete_statement = self.conn.prepare_cached(
            "DELETE FROM NAMING_SYMBOLS
            WHERE HASH = ? AND FILE_INFO_ID = ?",
        )?;
        let symbol_hash = ToplevelSymbolHash::new(item.name_type, &item.symbol);
        let canon_hash = ToplevelCanonSymbolHash::new(item.name_type, item.symbol.clone());
        let decl_hash = item.hash;
        let kind = item.name_type;

        if let Some(old) = self.get_row(symbol_hash)? {
            assert_eq!(symbol_hash, old.hash);
            assert_eq!(canon_hash, old.canon_hash);
            // check if new entry appears first alphabetically
            if path < &old.path {
                // delete old row from naming_symbols table
                delete_statement.execute(params![symbol_hash, old.file_info_id])?;

                // insert old row into overflow table
                insert_overflow_statement.execute(params![
                    symbol_hash,
                    canon_hash,
                    old.decl_hash,
                    old.kind,
                    old.file_info_id
                ])?;

                // insert new row into naming_symbols table
                insert_statement.execute(params![
                    symbol_hash,
                    canon_hash,
                    decl_hash,
                    kind,
                    file_info_id
                ])?;

                save_result
                    .checksum
                    .addremove(symbol_hash, old.decl_hash, &old.path); // remove old
                save_result.checksum.addremove(symbol_hash, decl_hash, path); // add new
                save_result.add_collision(kind, item.symbol, &old.path, path);
            } else {
                // insert new row into overflow table
                insert_overflow_statement.execute(params![
                    symbol_hash,
                    canon_hash,
                    decl_hash,
                    kind,
                    file_info_id
                ])?;
                save_result.add_collision(kind, item.symbol, &old.path, path);
            }
        } else {
            // No collision. Insert as you normally would
            insert_statement.execute(params![
                symbol_hash,
                canon_hash,
                decl_hash,
                kind,
                file_info_id
            ])?;
            save_result.checksum.addremove(symbol_hash, decl_hash, path);
            save_result.symbols_added += 1;
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
                let classes: Option<String> = row.get(0)?;
                let typedefs: Option<String> = row.get(1)?;
                Ok((classes, typedefs))
            })
            .optional()?;

        if let Some((classes, typedefs)) = names_opt {
            for class in classes.as_deref().unwrap_or_default().split_terminator('|') {
                if symbol_hash == ToplevelCanonSymbolHash::from_type(class.to_owned()) {
                    return Ok(Some(class.to_owned()));
                }
            }
            for typedef in typedefs
                .as_deref()
                .unwrap_or_default()
                .split_terminator('|')
            {
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
                let funs: Option<String> = row.get(0)?;
                Ok(funs)
            })
            .optional()?;

        if let Some(funs) = names_opt {
            for fun in funs.as_deref().unwrap_or_default().split_terminator('|') {
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
    pub fn get_symbol_hashes_for_winners(
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
    pub fn get_symbol_hashes_for_losers(
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

    /// This function will return an empty list if the path doesn't exist in the forward naming table
    pub fn get_symbol_hashes_for_winners_and_losers(
        &self,
        path: &RelativePath,
    ) -> anyhow::Result<Vec<(ToplevelSymbolHash, crate::FileInfoId)>> {
        // The NAMING_FILE_INFO table stores e.g. "Classname1|Classname2|Classname3". This
        // helper turns it into a vec of (ToplevelSymbolHash,file_info_id) pairs
        fn split(
            s: Option<String>,
            f: impl Fn(&str) -> ToplevelSymbolHash,
            file_info_id: crate::FileInfoId,
        ) -> Vec<(ToplevelSymbolHash, crate::FileInfoId)> {
            match s {
                None => vec![],
                Some(s) => s
                    .split_terminator('|')
                    .map(|name| (f(name), file_info_id))
                    .collect(),
            }
            // s.split_terminator yields an empty list for an empty string;
            // s.split would have yielded a singleton list, which we don't want.
        }

        let mut results = vec![];
        self.conn
            .prepare_cached(
                "SELECT CLASSES, CONSTS, FUNS, TYPEDEFS, MODULES, FILE_INFO_ID FROM NAMING_FILE_INFO
                WHERE PATH_PREFIX_TYPE = ?
                AND PATH_SUFFIX = ?",
            )?
            .query_row(params![path.prefix() as u8, path.path_str()], |row| {
                let file_info_id: crate::FileInfoId = row.get(5)?;
                results.extend(split(row.get(0)?, ToplevelSymbolHash::from_type, file_info_id));
                results.extend(split(row.get(1)?, ToplevelSymbolHash::from_const, file_info_id));
                results.extend(split(row.get(2)?, ToplevelSymbolHash::from_fun, file_info_id));
                results.extend(split(row.get(3)?, ToplevelSymbolHash::from_type, file_info_id));
                results.extend(split(row.get(4)?, ToplevelSymbolHash::from_module, file_info_id));
                Ok(())
            })
            .optional()?;
        Ok(results)
    }

    /// It looks up the forward-naming-table to find which symbols are in a filename,
    /// and for each it does a further lookup in the reverse-naming-table to find more about them.
    /// It only returns information about winners that were found in that filename.
    pub fn get_symbol_and_decl_hashes_for_winners(
        &self,
        path: &RelativePath,
    ) -> anyhow::Result<Vec<(ToplevelSymbolHash, DeclHash, crate::FileInfoId)>> {
        // For each symbol_hash we get from the forward-naming-table, look it up in the reverse-naming-table
        let mut results = vec![];
        for (symbol_hash, fwd_file_id) in self.get_symbol_hashes_for_winners_and_losers(path)? {
            self.conn
                .prepare_cached(
                    "SELECT DECL_HASH, FILE_INFO_ID FROM NAMING_SYMBOLS WHERE HASH = ?",
                )?
                .query_row(params![symbol_hash], |row| {
                    let decl_hash: DeclHash = row.get(0)?;
                    let winner_file_id: crate::FileInfoId = row.get(1)?;
                    if winner_file_id == fwd_file_id {
                        results.push((symbol_hash, decl_hash, winner_file_id));
                    }
                    Ok(())
                })?;
        }
        Ok(results)
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
                TYPEDEFS,
                MODULES
            )
            VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9);",
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
                Self::join_with_pipe(file_summary.modules()),
            ])?;
        let file_info_id = crate::FileInfoId::last_insert_rowid(&self.conn);
        Ok(file_info_id)
    }

    fn join_with_pipe<'a>(symbols: impl Iterator<Item = &'a crate::DeclSummary>) -> Option<String> {
        let s = symbols
            .map(|summary| summary.symbol.as_str())
            .collect::<Vec<_>>()
            .join("|");
        if s.is_empty() { None } else { Some(s) }
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

    /// This updates the forward naming table.
    /// It will replace the existing entry (preserving file_info_id) if file was present,
    /// or add a new entry (with new file_info_id) otherwise.
    /// It returns the file_info_id.
    /// Note: it never deletes a row.
    /// TODO(ljw): reconcile with existing delete() and insert_file_info_and_get_file_id()
    pub fn fwd_update(
        &self,
        path: &RelativePath,
        parsed_file: Option<&oxidized_by_ref::direct_decl_parser::ParsedFile<'_>>,
    ) -> anyhow::Result<crate::FileInfoId> {
        let file_info_id_opt = self
            .conn
            .prepare_cached(
                "SELECT FILE_INFO_ID FROM NAMING_FILE_INFO
                WHERE PATH_PREFIX_TYPE = ?
                AND PATH_SUFFIX = ?",
            )?
            .query_row(params![path.prefix() as u8, path.path_str()], |row| {
                row.get::<usize, crate::FileInfoId>(0)
            })
            .optional()?;

        let file_info_id = match file_info_id_opt {
            Some(file_info_id) => file_info_id,
            None => {
                self.conn
                .prepare_cached("INSERT INTO NAMING_FILE_INFO(PATH_PREFIX_TYPE,PATH_SUFFIX) VALUES (?1, ?2);")?
                .execute(params![
                    path.prefix() as u8,
                    path.path_str(),
                ])?;
                crate::FileInfoId::last_insert_rowid(&self.conn)
            }
        };

        // This helper takes a list of (name,decl) pairs and turns into string "name1|name2|..."
        fn join<'a, Decl, I: Iterator<Item = (&'a str, Decl)>>(i: I, sep: &'static str) -> String {
            i.map(|(name, _decl)| name).collect::<Vec<&str>>().join(sep)
        }

        let decls_or_empty = parsed_file
            .map_or_else(oxidized_by_ref::direct_decl_parser::Decls::empty, |pf| {
                pf.decls
            });
        self.conn
            .prepare_cached(
                "
                UPDATE NAMING_FILE_INFO
                SET TYPE_CHECKER_MODE=?, DECL_HASH=?, CLASSES=?, CONSTS=?, FUNS=?, TYPEDEFS=?, MODULES=?
                WHERE FILE_INFO_ID=?
                ",
            )?
            .execute(params![
                crate::datatypes::convert::mode_to_i64(parsed_file.and_then(|pf| pf.mode)),
                crate::hash_decls(&decls_or_empty),
                join(decls_or_empty.classes(), "|"),
                join(decls_or_empty.consts(), "|"),
                join(decls_or_empty.funs(), "|"),
                join(decls_or_empty.typedefs(), "|"),
                join(decls_or_empty.modules(), "|"),
                file_info_id,
            ])?;

        Ok(file_info_id)
    }

    /// This removes an entry from the forward naming table.
    /// TODO(ljw): reconcile with delete.
    pub fn fwd_delete(&self, file_info_id: crate::FileInfoId) -> anyhow::Result<()> {
        self.conn
            .prepare_cached("DELETE FROM NAMING_FILE_INFO WHERE FILE_INFO_ID = ?")?
            .execute(params![file_info_id])?;
        Ok(())
    }

    /// This updates the reverse naming-table NAMING_SYMBOLS and NAMING_SYMBOLS_OVERFLOW
    /// by removing all old entries, then inserting the new entries.
    /// TODO(ljw): remove previous implementations remove_symbol and insert_file_summary.
    pub fn rev_update(
        &self,
        symbol_hash: ToplevelSymbolHash,
        winner: Option<&crate::SymbolRow>,
        overflow: &[&crate::SymbolRow],
    ) -> anyhow::Result<()> {
        self.conn
            .prepare("DELETE FROM NAMING_SYMBOLS WHERE HASH = ?")?
            .execute(params![symbol_hash])?;
        self.conn
            .prepare("DELETE FROM NAMING_SYMBOLS_OVERFLOW WHERE HASH = ?")?
            .execute(params![symbol_hash])?;
        if let Some(symbol) = winner {
            self.conn.prepare("INSERT INTO NAMING_SYMBOLS (HASH, CANON_HASH, DECL_HASH, FLAGS, FILE_INFO_ID) VALUES (?,?,?,?,?)")?
            .execute(params![
                symbol.hash,
                symbol.canon_hash,
                symbol.decl_hash,
                symbol.kind,
                symbol.file_info_id
            ])?;
        }
        for symbol in overflow {
            self.conn.prepare("INSERT INTO NAMING_SYMBOLS_OVERFLOW (HASH, CANON_HASH, DECL_HASH, FLAGS, FILE_INFO_ID) VALUES (?,?,?,?,?)")?
            .execute(params![
                symbol.hash,
                symbol.canon_hash,
                symbol.decl_hash,
                symbol.kind,
                symbol.file_info_id
            ])?;
        }

        Ok(())
    }

    pub fn build_at_path(
        path: impl AsRef<Path>,
        file_summaries: impl IntoIterator<Item = (RelativePath, crate::FileSummary)>,
    ) -> anyhow::Result<Self> {
        let path = path.as_ref();
        let conn = Connection::open(path)?;
        let log = slog::Logger::root(slog::Discard, slog::o!());
        let (conn, _save_result) = Self::build(&log, conn, |tx| {
            file_summaries.into_iter().try_for_each(|x| Ok(tx.send(x)?))
        })?;
        Ok(Self { conn })
    }

    /// Build a naming table using the information provided in
    /// `collect_file_summaries` and writing to `conn`. The naming table will be
    /// built on a background thread while `collect_file_summaries` is run. Once
    /// all file summaries have been sent on the `Sender`, drop it (usually by
    /// letting it go out of scope as `collect_file_summaries` terminates) to
    /// allow building to continue.
    pub fn build(
        log: &slog::Logger,
        mut conn: rusqlite::Connection,
        collect_file_summaries: impl FnOnce(
            crossbeam::channel::Sender<(RelativePath, crate::FileSummary)>,
        ) -> anyhow::Result<()>,
    ) -> anyhow::Result<(rusqlite::Connection, crate::SaveResult)> {
        // We can't use rusqlite::Transaction for now because a lot of methods
        // we want to use are on Self, and Self wants ownership of the
        // Connection. Sqlite will automatically perform a rollback when the
        // connection is closed, which will happen (via impl Drop for
        // Connection) if we return Err here.
        conn.execute("BEGIN TRANSACTION", params![])?;
        Self::create_tables(&mut conn)?;

        let mut names = Self { conn };
        let save_result = crossbeam::thread::scope(|scope| -> anyhow::Result<_> {
            let (tx, rx) = crossbeam::channel::unbounded();

            // Write to the db serially, but concurrently with parsing
            let names = &mut names;
            let db_thread = scope.spawn(move |_| -> anyhow::Result<_> {
                let mut save_result = crate::SaveResult::default();
                while let Ok((path, summary)) = rx.recv() {
                    names.insert_file_summary(&path, &summary, &mut save_result)?;
                }
                Ok(save_result)
            });

            // Parse files (in parallel, if the caller chooses)
            collect_file_summaries(tx)?;

            db_thread.join().unwrap()
        })
        .unwrap()?;
        names.set_checksum(save_result.checksum)?;
        let mut conn = names.conn;

        slog::info!(log, "Creating indices...");
        Self::create_indices(&mut conn)?;

        slog::info!(log, "Closing DB transaction...");
        conn.execute("END TRANSACTION", params![])?;

        Ok((conn, save_result))
    }
}
