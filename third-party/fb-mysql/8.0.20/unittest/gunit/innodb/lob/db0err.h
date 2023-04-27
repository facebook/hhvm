/*****************************************************************************

Copyright (c) 1996, 2019, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/**************************************************/ /**
 @file include/db0err.h
 Global error codes for the database

 Created 5/24/1996 Heikki Tuuri
 *******************************************************/

#ifndef db0err_h
#define db0err_h

/* Do not include univ.i because univ.i includes this. */

enum dberr_t {
  DB_SUCCESS_LOCKED_REC = 9, /*!< like DB_SUCCESS, but a new
                             explicit record lock was created */
  DB_SUCCESS = 10,

  /* The following are error codes */
  DB_ERROR,
  DB_INTERRUPTED,
  DB_OUT_OF_MEMORY,
  DB_OUT_OF_FILE_SPACE,
  DB_LOCK_WAIT,
  DB_DEADLOCK,
  DB_ROLLBACK,
  DB_DUPLICATE_KEY,
  DB_MISSING_HISTORY, /*!< required history data has been
                      deleted due to lack of space in
                      rollback segment */
  DB_CLUSTER_NOT_FOUND = 30,
  DB_TABLE_NOT_FOUND,
  DB_MUST_GET_MORE_FILE_SPACE, /*!< the database has to be stopped
                               and restarted with more file space */
  DB_TABLE_IS_BEING_USED,
  DB_TOO_BIG_RECORD,           /*!< a record in an index would not fit
                               on a compressed page, or it would
                               become bigger than 1/2 free space in
                               an uncompressed page frame */
  DB_LOCK_WAIT_TIMEOUT,        /*!< lock wait lasted too long */
  DB_NO_REFERENCED_ROW,        /*!< referenced key value not found
                               for a foreign key in an insert or
                               update of a row */
  DB_ROW_IS_REFERENCED,        /*!< cannot delete or update a row
                               because it contains a key value
                               which is referenced */
  DB_CANNOT_ADD_CONSTRAINT,    /*!< adding a foreign key constraint
                               to a table failed */
  DB_CORRUPTION,               /*!< data structure corruption
                               noticed */
  DB_CANNOT_DROP_CONSTRAINT,   /*!< dropping a foreign key constraint
                               from a table failed */
  DB_NO_SAVEPOINT,             /*!< no savepoint exists with the given
                               name */
  DB_TABLESPACE_EXISTS,        /*!< we cannot create a new single-table
                               tablespace because a file of the same
                               name already exists */
  DB_TABLESPACE_DELETED,       /*!< tablespace was deleted or is
                               being dropped right now */
  DB_TABLESPACE_NOT_FOUND,     /*!< Attempt to delete a tablespace
                               instance that was not found in the
                               tablespace hash table */
  DB_LOCK_TABLE_FULL,          /*!< lock structs have exhausted the
                               buffer pool (for big transactions,
                               InnoDB stores the lock structs in the
                               buffer pool) */
  DB_FOREIGN_DUPLICATE_KEY,    /*!< foreign key constraints
                               activated by the operation would
                               lead to a duplicate key in some
                               table */
  DB_TOO_MANY_CONCURRENT_TRXS, /*!< when InnoDB runs out of the
                               preconfigured undo slots, this can
                               only happen when there are too many
                               concurrent transactions */
  DB_UNSUPPORTED,              /*!< when InnoDB sees any artefact or
                               a feature that it can't recoginize or
                               work with e.g., FT indexes created by
                               a later version of the engine. */

  DB_INVALID_NULL, /*!< a NOT NULL column was found to
                   be NULL during table rebuild */

  DB_STATS_DO_NOT_EXIST,         /*!< an operation that requires the
                                 persistent storage, used for recording
                                 table and index statistics, was
                                 requested but this storage does not
                                 exist itself or the stats for a given
                                 table do not exist */
  DB_FOREIGN_EXCEED_MAX_CASCADE, /*!< Foreign key constraint related
                                 cascading delete/update exceeds
                                 maximum allowed depth */
  DB_CHILD_NO_INDEX,             /*!< the child (foreign) table does
                                 not have an index that contains the
                                 foreign keys as its prefix columns */
  DB_PARENT_NO_INDEX,            /*!< the parent table does not
                                 have an index that contains the
                                 foreign keys as its prefix columns */
  DB_TOO_BIG_INDEX_COL,          /*!< index column size exceeds
                                 maximum limit */
  DB_INDEX_CORRUPT,              /*!< we have corrupted index */
  DB_UNDO_RECORD_TOO_BIG,        /*!< the undo log record is too big */
  DB_READ_ONLY,                  /*!< Update operation attempted in
                                 a read-only transaction */
  DB_FTS_INVALID_DOCID,          /* FTS Doc ID cannot be zero */
  DB_TABLE_IN_FK_CHECK,          /* table is being used in foreign
                                 key check */
  DB_ONLINE_LOG_TOO_BIG,         /*!< Modification log grew too big
                                 during online index creation */

  DB_IDENTIFIER_TOO_LONG,           /*!< Identifier name too long */
  DB_FTS_EXCEED_RESULT_CACHE_LIMIT, /*!< FTS query memory
                            exceeds result cache limit */
  DB_TEMP_FILE_WRITE_FAIL,          /*!< Temp file write failure */
  DB_CANT_CREATE_GEOMETRY_OBJECT,   /*!< Cannot create specified Geometry
                                    data object */
  DB_CANNOT_OPEN_FILE,              /*!< Cannot open a file */
  DB_FTS_TOO_MANY_WORDS_IN_PHRASE,
  /*!< Too many words in a phrase */

  DB_IO_ERROR = 100, /*!< Generic IO error */

  DB_IO_DECOMPRESS_FAIL, /*!< Failure to decompress a page
                         after reading it from disk */

  DB_IO_NO_PUNCH_HOLE, /*!< Punch hole not supported by
                       InnoDB */

  DB_IO_NO_PUNCH_HOLE_FS, /*!< The file system doesn't support
                          punch hole */

  DB_IO_NO_PUNCH_HOLE_TABLESPACE, /*!< The tablespace doesn't support
                                  punch hole */

  DB_IO_DECRYPT_FAIL, /*!< Failure to decrypt a page
                      after reading it from disk */

  DB_IO_NO_ENCRYPT_TABLESPACE, /*!< The tablespace doesn't support
                               encrypt */

  DB_IO_PARTIAL_FAILED, /*!< Partial IO request failed */

  DB_FORCED_ABORT, /*!< Transaction was forced to rollback
                   by a higher priority transaction */

  DB_TABLE_CORRUPT, /*!< Table/clustered index is
                    corrupted */

  DB_WRONG_FILE_NAME, /*!< Invalid Filename */

  DB_COMPUTE_VALUE_FAILED, /*!< Compute generated value failed */
  DB_NO_FK_ON_S_BASE_COL,  /*!< Cannot add foreign constrain
                           placed on the base column of
                           stored column */

  DB_INVALID_ENCRYPTION_META, /*!< Invalid encrytion metadata in
                              page 0. */

  /* The following are partial failure codes */
  DB_FAIL = 1000,
  DB_OVERFLOW,
  DB_UNDERFLOW,
  DB_STRONG_FAIL,
  DB_ZIP_OVERFLOW,
  DB_RECORD_NOT_FOUND = 1500,
  DB_END_OF_INDEX,
  DB_END_SAMPLE_READ,
  DB_NOT_FOUND, /*!< Generic error code for "Not found"
                type of errors */

  /* The following are API only error codes. */
  DB_DATA_MISMATCH = 2000, /*!< Column update or read failed
                           because the types mismatch */
};

#endif
