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

    pub fn from_file_readonly(path: impl AsRef<Path>) -> anyhow::Result<Self> {
        use rusqlite::OpenFlags;
        let path = path.as_ref();
        let conn = Connection::open_with_flags(
            path,
            OpenFlags::SQLITE_OPEN_READ_ONLY | OpenFlags::SQLITE_OPEN_NO_MUTEX,
        )?;
        let this = Self { conn };
        this.pragma_fast_but_not_durable()?;
        Ok(this)
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

    /// These pragmas make things faster at the expense of write safety...
    /// * journal_mode=OFF -- no rollback possible: the ROLLBACK TRANSACTION command won't work
    /// * synchronous=OFF -- sqlite will return immediately after handing off writes to the OS, so data will be lost upon power-loss
    /// * temp_store=MEMORY -- temporary tables and indices kept in memory
    pub fn pragma_fast_but_not_durable(&self) -> anyhow::Result<()> {
        self.conn.execute_batch(
            "PRAGMA journal_mode = OFF;
            PRAGMA synchronous = OFF;
            PRAGMA temp_store = MEMORY;",
        )?;
        Ok(())
    }

    /// This does a sql "BEGIN EXCLUSIVE TRANSACTION".
    /// (an 'exclusive' transaction is one that acquires a write lock immediately rather than lazily).
    /// Then, once you call transaction.end() or drop it, "END TRANSACTION".
    /// The main reason to use transactions is for speed.
    /// (Note: if you opened naming-sqlite with "from_file_non_durable" then it
    /// doesn't support ROLLBACK TRANSACTION).
    pub fn transaction(&self) -> anyhow::Result<Transaction<'_>> {
        Transaction::new(&self.conn)
    }

    fn create_tables(conn: &mut Connection) -> anyhow::Result<()> {
        conn.execute(
            "
            CREATE TABLE IF NOT EXISTS naming_symbols (
                hash INTEGER PRIMARY KEY NOT NULL,
                canon_hash INTEGER NOT NULL,
                decl_hash INTEGER NOT NULL,
                flags INTEGER NOT NULL,
                file_info_id INTEGER NOT NULL,
                name TEXT NOT NULL,
                sort_text TEXT NULL
            );",
            params![],
        )?;

        conn.execute(
            "
            CREATE TABLE IF NOT EXISTS naming_symbols_overflow (
                hash INTEGER PRIMARY KEY NOT NULL,
                canon_hash INTEGER NOT NULL,
                decl_hash INTEGER NOT NULL,
                flags INTEGER NOT NULL,
                file_info_id INTEGER NOT NULL,
                name TEXT NOT NULL,
                sort_text TEXT NULL
            );",
            params![],
        )?;

        conn.execute(
            "
            CREATE TABLE IF NOT EXISTS naming_file_info (
                file_info_id INTEGER PRIMARY KEY AUTOINCREMENT,
                file_digest TEXT,
                path_prefix_type INTEGER NOT NULL,
                path_suffix TEXT NOT NULL,
                type_checker_mode INTEGER,
                decl_hash INTEGER,
                classes TEXT,
                consts TEXT,
                funs TEXT,
                typedefs TEXT,
                modules TEXT
            );",
            params![],
        )?;

        conn.execute(
            "
            CREATE TABLE IF NOT EXISTS checksum (
                id INTEGER PRIMARY KEY,
                checksum_value INTEGER NOT NULL
            );
            ",
            params![],
        )?;

        conn.execute(
            "INSERT OR IGNORE INTO checksum (id, checksum_value) VALUES (0, 0);",
            params![],
        )?;

        // This table contains a single dummy value and is here only to satisfy
        // hh_server and hh_single_type_check.
        conn.execute(
            "
            CREATE TABLE IF NOT EXISTS naming_local_changes(
                id INTEGER PRIMARY KEY,
                local_changes BLOB NOT NULL,
                base_content_version TEXT
            );",
            params![],
        )?;

        // The blob here is Relative_path.Map.empty as an OCaml-marshaled blob.
        // The base_content_version (computed from the unix timestamp and a
        // random ID) needs only be unique.
        conn.execute(
            "
            INSERT OR IGNORE INTO naming_local_changes
            VALUES(0,X'8495a6be0000000100000000000000000000000040',?);",
            params![format!(
                "{}-{}",
                std::time::SystemTime::now()
                    .duration_since(std::time::SystemTime::UNIX_EPOCH)
                    .expect("SystemTime::now() before UNIX_EPOCH")
                    .as_secs(),
                {
                    use rand::distributions::DistString;
                    rand::distributions::Alphanumeric.sample_string(&mut rand::thread_rng(), 10)
                },
            )],
        )?;

        Ok(())
    }

    pub fn create_indices(conn: &mut Connection) -> anyhow::Result<()> {
        conn.execute(
            "CREATE UNIQUE INDEX IF NOT EXISTS file_info_path_idx
             ON naming_file_info (path_suffix, path_prefix_type);",
            params![],
        )?;

        conn.execute(
            "CREATE INDEX IF NOT EXISTS types_canon
             ON naming_symbols (canon_hash);",
            params![],
        )?;

        Ok(())
    }

    pub fn get_checksum(&self) -> anyhow::Result<Checksum> {
        Ok(self
            .conn
            .prepare_cached("SELECT checksum_value FROM checksum")?
            .query_row(params![], |row| row.get(0))?)
    }

    pub fn set_checksum(&self, checksum: Checksum) -> anyhow::Result<()> {
        self.conn
            .prepare_cached("REPLACE INTO checksum (id, checksum_value) VALUES (0, ?);")?
            .execute(params![checksum])?;
        Ok(())
    }

    // helper for `save_file_summary`/`build`
    pub fn insert_file_summary(
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
            "INSERT INTO naming_symbols (hash, canon_hash, decl_hash, flags, file_info_id, name, sort_text)
            VALUES (?, ?, ?, ?, ?, ?, ?);",
        )?;
        let mut insert_overflow_statement = self.conn.prepare_cached(
            "INSERT INTO naming_symbols_overflow (hash, canon_hash, decl_hash, flags, file_info_id, name, sort_text)
            VALUES (?, ?, ?, ?, ?, ?, ?);",
        )?;
        let mut delete_statement = self.conn.prepare_cached(
            "DELETE FROM naming_symbols
            WHERE hash = ? AND file_info_id = ?",
        )?;
        let symbol_hash = ToplevelSymbolHash::new(item.name_type, &item.symbol);
        let canon_hash = ToplevelCanonSymbolHash::new(item.name_type, item.symbol.clone());
        let decl_hash = item.hash;
        let kind = item.name_type;
        let name = &item.symbol;
        let sort_text = &item.symbol; // should probably read sort_text off the declaration/attribute

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
                    old.file_info_id,
                    old.name,
                    old.sort_text
                ])?;

                // insert new row into naming_symbols table
                insert_statement.execute(params![
                    symbol_hash,
                    canon_hash,
                    decl_hash,
                    kind,
                    file_info_id,
                    name,
                    sort_text
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
                    file_info_id,
                    name,
                    sort_text
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
                file_info_id,
                name,
                sort_text
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
    ) -> anyhow::Result<Vec<crate::SymbolRowNew>> {
        let select_statement = "
        SELECT
            naming_symbols_overflow.hash,
            naming_symbols_overflow.canon_hash,
            naming_symbols_overflow.decl_hash,
            naming_symbols_overflow.flags,
            naming_symbols_overflow.file_info_id,
            namimg_symbols_overflow.name,
            naming_symbols_overflow.sort_text,
            naming_file_info.path_prefix_type,
            naming_file_info.path_suffix
        FROM
            naming_symbols_overflow
        LEFT JOIN
            naming_file_info
        ON
            naming_symbols_overflow.file_info_id = naming_file_info.file_info_id
        WHERE
            naming_symbols_overflow.hash = ?
        ";

        let mut select_statement = self.conn.prepare_cached(select_statement)?;
        let mut rows = select_statement.query(params![symbol_hash])?;
        let mut result = vec![];
        while let Some(row) = rows.next()? {
            let prefix: crate::datatypes::SqlitePrefix = row.get(7)?;
            let suffix: crate::datatypes::SqlitePathBuf = row.get(8)?;
            let path = RelativePath::make(prefix.value, suffix.value);
            result.push(crate::SymbolRowNew {
                hash: row.get(0)?,
                canon_hash: row.get(1)?,
                decl_hash: row.get(2)?,
                kind: row.get(3)?,
                file_info_id: row.get(4)?,
                name: row.get(5)?,
                sort_text: row.get(6)?,
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
    ) -> anyhow::Result<Option<crate::SymbolRowNew>> {
        let select_statement = "
        SELECT
            naming_symbols.hash,
            naming_symbols.canon_hash,
            naming_symbols.decl_hash,
            naming_symbols.flags,
            naming_symbols.file_info_id,
            naming_symbols.name,
            naming_symbols.sort_text,
            naming_file_info.path_prefix_type,
            naming_file_info.path_suffix
        FROM
        naming_symbols
        LEFT JOIN
            naming_file_info
        ON
            naming_symbols.file_info_id = naming_file_info.file_info_id
        WHERE
            naming_symbols.hash = ?
        ";

        let mut select_statement = self.conn.prepare_cached(select_statement)?;
        let result = select_statement
            .query_row(params![symbol_hash], |row| {
                let prefix: crate::datatypes::SqlitePrefix = row.get(7)?;
                let suffix: crate::datatypes::SqlitePathBuf = row.get(8)?;
                let path = RelativePath::make(prefix.value, suffix.value);
                Ok(crate::SymbolRowNew {
                    hash: row.get(0)?,
                    canon_hash: row.get(1)?,
                    decl_hash: row.get(2)?,
                    kind: row.get(3)?,
                    file_info_id: row.get(4)?,
                    name: row.get(5)?,
                    sort_text: row.get(6)?,
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
            .prepare_cached("SELECT decl_hash FROM naming_symbols WHERE hash = ?")?
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
            naming_file_info.path_prefix_type,
            naming_file_info.path_suffix,
            naming_symbols.flags
        FROM
            naming_symbols
        LEFT JOIN
            naming_file_info
        ON
            naming_symbols.file_info_id = naming_file_info.file_info_id
        WHERE
            naming_symbols.hash = ?
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
            naming_file_info.path_prefix_type,
            naming_file_info.path_suffix
        FROM
            naming_symbols
        LEFT JOIN
            naming_file_info
        ON
            naming_symbols.file_info_id = naming_file_info.file_info_id
        WHERE
        naming_symbols.canon_hash = ?
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
            naming_file_info.classes,
            naming_file_info.typedefs
        FROM
            naming_symbols
        LEFT JOIN
            naming_file_info
        ON
            naming_symbols.file_info_id = naming_file_info.file_info_id
        WHERE
            naming_symbols.canon_hash = ?
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
            naming_file_info.FUNS
        FROM
            naming_symbols
        LEFT JOIN
            naming_file_info
        ON
            naming_symbols.file_info_id = naming_file_info.file_info_id
        WHERE
            naming_symbols.canon_hash = ?
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

    /// This function will return an empty list if the path doesn't exist in the forward naming table
    pub fn get_symbol_hashes_for_winners_and_losers(
        &self,
        path: &RelativePath,
    ) -> anyhow::Result<Vec<(ToplevelSymbolHash, crate::FileInfoId)>> {
        // The naming_file_info table stores e.g. "Classname1|Classname2|Classname3". This
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
                "SELECT classes, consts, funs, typedefs, modules, file_info_id FROM naming_file_info
                WHERE path_prefix_type = ?
                AND path_suffix = ?",
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

    /// This inserts an item into the forward naming table.
    fn insert_file_info_and_get_file_id(
        &self,
        path_rel: &RelativePath,
        file_summary: &crate::FileSummary,
    ) -> anyhow::Result<crate::FileInfoId> {
        let prefix_type = path_rel.prefix() as u8; // TODO(ljw): shouldn't this use prefix_to_i64?
        let suffix = path_rel.path().to_str().unwrap();
        let type_checker_mode = crate::datatypes::convert::mode_to_i64(file_summary.mode);
        let file_decls_hash = file_summary.file_decls_hash;
        let file_digest = match &file_summary.file_digest {
            Some(digest) => digest,
            None => "",
        };

        self.conn
            .prepare_cached(
                "INSERT INTO naming_file_info(
                file_digest,
                path_prefix_type,
                path_suffix,
                type_checker_mode,
                decl_hash,
                classes,
                consts,
                funs,
                typedefs,
                modules
            )
            VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10);",
            )?
            .execute(params![
                file_digest,
                prefix_type,
                suffix,
                type_checker_mode,
                file_decls_hash,
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

    /// This updates the forward naming table.
    /// It will replace the existing entry (preserving file_info_id) if file was present,
    /// or add a new entry (with new file_info_id) otherwise.
    /// It returns the file_info_id.
    /// Note: it never deletes a row.
    /// TODO(ljw): reconcile with existing delete() and insert_file_info_and_get_file_id()
    pub fn fwd_update(
        &self,
        path: &RelativePath,
        file_summary: Option<&crate::FileSummary>,
    ) -> anyhow::Result<crate::FileInfoId> {
        let file_info_id_opt = self
            .conn
            .prepare_cached(
                "SELECT file_info_id FROM naming_file_info
                WHERE path_prefix_type = ?
                AND path_suffix = ?",
            )?
            .query_row(params![path.prefix() as u8, path.path_str()], |row| {
                row.get::<usize, crate::FileInfoId>(0)
            })
            .optional()?;

        let file_info_id = match file_info_id_opt {
            Some(file_info_id) => file_info_id,
            None => {
                self.conn
                .prepare_cached("INSERT INTO naming_file_info(path_prefix_type,path_suffix) VALUES (?1, ?2);")?
                .execute(params![
                    path.prefix() as u8,
                    path.path_str(),
                ])?;
                crate::FileInfoId::last_insert_rowid(&self.conn)
            }
        };

        let _a = file_summary.and_then(|fs| Self::join_with_pipe(fs.classes()));
        self.conn
            .prepare_cached(
                "
                UPDATE naming_file_info
                SET type_checker_mode=?, decl_hash=?, classes=?, consts=?, funs=?, typedefs=?, modules=?
                WHERE file_info_id=?
                ",
            )?
            .execute(params![
                crate::datatypes::convert::mode_to_i64(file_summary.and_then(|fs| fs.mode)),
                file_summary.map(|fs| fs.file_decls_hash),
                file_summary.and_then(|fs| Self::join_with_pipe(fs.classes())),
                file_summary.and_then(|fs| Self::join_with_pipe(fs.consts())),
                file_summary.and_then(|fs| Self::join_with_pipe(fs.funs())),
                file_summary.and_then(|fs| Self::join_with_pipe(fs.typedefs())),
                file_summary.and_then(|fs| Self::join_with_pipe(fs.modules())),
                file_info_id,
            ])?;

        Ok(file_info_id)
    }

    /// Wrapper around `build` (see its documentation); this wrapper is for when you
    /// want to pass file summaries as an iterator rather than send them over a channel.
    pub fn build_from_iterator(
        path: impl AsRef<Path>,
        file_summaries: impl IntoIterator<Item = (RelativePath, crate::FileSummary)>,
    ) -> anyhow::Result<crate::SaveResult> {
        Self::build(path, |tx| {
            file_summaries.into_iter().try_for_each(|x| Ok(tx.send(x)?))
        })
    }

    /// Build a naming table at `path` (its containing directory will be created if necessary)
    /// out of the information provided in `collect_file_summaries`. The naming table will be
    /// built on a background thread while `collect_file_summaries` is run. Once
    /// all file summaries have been sent on the `Sender` , drop it (usually by
    /// letting it go out of scope as `collect_file_summaries` terminates) to
    /// allow building to continue.
    /// If this function errors, the naming-table at `path` will be left in an indeterminate state.
    pub fn build(
        path: impl AsRef<Path>,
        collect_file_summaries: impl FnOnce(
            crossbeam::channel::Sender<(RelativePath, crate::FileSummary)>,
        ) -> anyhow::Result<()>,
    ) -> anyhow::Result<crate::SaveResult> {
        let path = path.as_ref();
        std::fs::create_dir_all(
            path.parent()
                .ok_or_else(|| anyhow::anyhow!("Invalid output path: {}", path.display()))?,
        )?;
        let mut conn = Connection::open(path)?;

        // The upshot of these commands is they're the fastest way we've found to write
        // millions of rows into sqlite. They avoid sqlite having to acquire locks each
        // row, and avoid it facing the overhead of recoverability in case of failure.
        // The idea of a "transaction" here is a bit odd, because we're non-recoverable,
        // in case of error, but it's part of the magic sauce of helping sqlite go fast
        // (since it avoids the overhead of starting a fresh transaction once per row).
        conn.execute_batch(
            "PRAGMA journal_mode = OFF;
        PRAGMA synchronous = OFF;
        PRAGMA cache_size = 1000000;
        PRAGMA locking_mode = EXCLUSIVE;
        PRAGMA temp_store = MEMORY;
        BEGIN TRANSACTION;",
        )?;

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

        // Another part of the magic sauce for sqlite speed is to create indices in
        // one go at the end, rather than having them updated once per row.
        let mut conn = names.conn;
        Self::create_indices(&mut conn)?;

        conn.execute_batch("END TRANSACTION; VACUUM;")?;
        conn.close().map_err(|(_conn, e)| e)?;

        Ok(save_result)
    }

    /// Close the connection. If the close fails, returns Err(conn, err)
    /// so the caller can try again, report what went wrong, etc.
    /// drop() has the same flushing behavior as close(), but without error handling.
    pub fn close(self) -> Result<(), (rusqlite::Connection, rusqlite::Error)> {
        self.conn.close()
    }
}

/// This token is for a transaction. When you construct it,
/// it does sql command "BEGIN EXCLUSIVE TRANSACTION".
/// When you call end() or drop it, it does sql command "END TRANSACTION".
/// (There's also a similar rusqlite::Transaction, but it takes &mut
/// ownership of the connection, which makes it awkward to work with
/// all our methods.)
pub struct Transaction<'a> {
    conn: Option<&'a rusqlite::Connection>,
}

impl<'a> Transaction<'a> {
    fn new(conn: &'a rusqlite::Connection) -> anyhow::Result<Self> {
        conn.execute_batch("BEGIN EXCLUSIVE TRANSACTION;")?;
        Ok(Self { conn: Some(conn) })
    }

    pub fn end(mut self) -> anyhow::Result<()> {
        if let Some(conn) = self.conn.take() {
            conn.execute_batch("END TRANSACTION;")?;
        }
        Ok(())
    }
}

impl<'a> Drop for Transaction<'a> {
    fn drop(&mut self) {
        if let Some(conn) = self.conn.take() {
            let _ignore = conn.execute_batch("END TRANSACTION;");
        }
    }
}
