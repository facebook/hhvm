/**
 *  Copyright 2009-2013 10gen, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include "php.h"
#include "php_globals.h"
#include "zend_exceptions.h"
#include "ext/standard/php_smart_str.h"

#include "../php_mongo.h"
#include "../collection.h"
#include "../cursor.h"
#include "../db.h"
#include "gridfs_cursor.h"
#include "gridfs_file.h"
#include "../types/bin_data.h"
#include "../types/date.h"
#include "../types/id.h"

ZEND_EXTERN_MODULE_GLOBALS(mongo)

extern zend_class_entry *mongo_ce_BinData;
extern zend_class_entry *mongo_ce_Collection;
extern zend_class_entry *mongo_ce_Date;
extern zend_class_entry *mongo_ce_DB;
extern zend_class_entry *mongo_ce_Exception;
extern zend_class_entry *mongo_ce_GridFSCursor;
extern zend_class_entry *mongo_ce_GridFSException;
extern zend_class_entry *mongo_ce_GridFSFile;
extern zend_class_entry *mongo_ce_Id;

zend_class_entry *mongo_ce_GridFS = NULL;

typedef struct {
	FILE *file;
	int fd;                     /* underlying file descriptor */
	unsigned is_process_pipe:1; /* use pclose instead of fclose */
	unsigned is_pipe:1;         /* don't try and seek */
	unsigned cached_fstat:1;    /* sb is valid */
	unsigned _reserved:29;

	int lock_flag;              /* stores the lock state */
	char *temp_file_name;       /* if non-null, this is the path to a temporary file that
	                             * is to be deleted when the stream is closed */
#if HAVE_FLUSHIO
	char last_op;
#endif

#if HAVE_MMAP
	char *last_mapped_addr;
	size_t last_mapped_len;
#endif
#ifdef PHP_WIN32
	char *last_mapped_addr;
	HANDLE file_mapping;
#endif

	struct stat sb;
} php_stdio_stream_data;

static int setup_file_fields(zval *zfile, char *filename, int size TSRMLS_DC);
static zval* insert_chunk(zval *chunks, zval *zid, int chunk_num, char *buf, int chunk_size, zval *options TSRMLS_DC);

/* {{{ proto MongoGridFS::__construct(MongoDB db [, string prefix = "fs"])
   Creates a new MongoGridFS object */
PHP_METHOD(MongoGridFS, __construct)
{
	zval *zdb, *files = NULL, *chunks = NULL, *zchunks;
	zval *z_w = NULL;

	/* chunks is deprecated */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|zz", &zdb, mongo_ce_DB, &files, &chunks) == FAILURE) {
		zval *object = getThis();
		ZVAL_NULL(object);
		return;
	}

	if (chunks) {
		php_error_docref(NULL TSRMLS_CC, MONGO_E_DEPRECATED, "The 'chunks' argument is deprecated and ignored");
	}

	if (files) {
		zval *temp_file;
		char *temp;

		if (Z_TYPE_P(files) != IS_STRING || Z_STRLEN_P(files) == 0 ) {
			zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 2 TSRMLS_CC, "MongoGridFS::__construct(): invalid prefix");
			return;
		}

		MAKE_STD_ZVAL(chunks);
		spprintf(&temp, 0, "%s.chunks", Z_STRVAL_P(files));
		ZVAL_STRING(chunks, temp, 0);

		MAKE_STD_ZVAL(temp_file);
		spprintf(&temp, 0, "%s.files", Z_STRVAL_P(files));
		ZVAL_STRING(temp_file, temp, 0);
		files = temp_file;
	} else {
		MAKE_STD_ZVAL(files);
		ZVAL_STRING(files, "fs.files", 1);
		MAKE_STD_ZVAL(chunks);
		ZVAL_STRING(chunks, "fs.chunks", 1);
	}

	/* create files collection */
	MONGO_METHOD2(MongoCollection, __construct, return_value, getThis(), zdb, files);

	/* create chunks collection */
	MAKE_STD_ZVAL(zchunks);
	object_init_ex(zchunks, mongo_ce_Collection);
	MONGO_METHOD2(MongoCollection, __construct, return_value, zchunks, zdb, chunks);

	/* add chunks collection as a property */
	zend_update_property(mongo_ce_GridFS, getThis(), "chunks", strlen("chunks"), zchunks TSRMLS_CC);
	zend_update_property(mongo_ce_GridFS, getThis(), "filesName", strlen("filesName"), files TSRMLS_CC);
	zend_update_property(mongo_ce_GridFS, getThis(), "chunksName", strlen("chunksName"), chunks TSRMLS_CC);

	/* GridFS is forced in our codebase to be w=1 so this property doesn't actually mean
	 * anything, but we can't lie to the user so we have to overwrite it if the MongoDB
	 * object that created this object was w=0.
	 * This property is initialized in the MongoCollection (which we extend) ctor */
	z_w = zend_read_property(mongo_ce_GridFS, getThis(), "w", strlen("w"), NOISY TSRMLS_CC);
	if (Z_TYPE_P(z_w) != IS_STRING) {
		convert_to_long(z_w);
		if (Z_LVAL_P(z_w) < 2) {
			zend_update_property_long(mongo_ce_GridFS, getThis(), "w", strlen("w"), 1 TSRMLS_CC);
		}
	}

	/* cleanup */
	zval_ptr_dtor(&zchunks);

	zval_ptr_dtor(&files);
	zval_ptr_dtor(&chunks);
}
/* }}} */

void php_mongo_ensure_gridfs_index(zval *return_value, zval *this_ptr TSRMLS_DC)
{
	zval *index, *options;

	/* ensure index on chunks.n */
	MAKE_STD_ZVAL(index);
	array_init(index);
	add_assoc_long(index, "files_id", 1);
	add_assoc_long(index, "n", 1);

	MAKE_STD_ZVAL(options);
	array_init(options);
	add_assoc_bool(options, "unique", 1);
	add_assoc_bool(options, "dropDups", 1);

	MONGO_METHOD2(MongoCollection, ensureIndex, return_value, getThis(), index, options);

	zval_ptr_dtor(&index);
	zval_ptr_dtor(&options);
}

/* {{{ proto array MongoGridFS::drop()
   Drops the files and chunks collections */
PHP_METHOD(MongoGridFS, drop)
{
	zval *temp;
	zval *zchunks = zend_read_property(mongo_ce_GridFS, getThis(), "chunks", strlen("chunks"), NOISY TSRMLS_CC);

	MAKE_STD_ZVAL(temp);
	MONGO_METHOD(MongoCollection, drop, temp, zchunks);
	zval_ptr_dtor(&temp);

	MONGO_METHOD(MongoCollection, drop, return_value, getThis());
}
/* }}} */

/* {{{ proto MongoGridFSCursor MongoGridFS::find([array|object query = array() [, array fields = array()]])
   Queries for files */
PHP_METHOD(MongoGridFS, find)
{
	zval temp;
	zval *zquery = 0, *zfields = 0;
	mongo_collection *c;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zz", &zquery, &zfields) == FAILURE) {
		return;
	}
	MUST_BE_ARRAY_OR_OBJECT(1, zquery);
	MUST_BE_ARRAY_OR_OBJECT(2, zfields);

	if (!zquery) {
		MAKE_STD_ZVAL(zquery);
		array_init(zquery);
	} else {
		zval_add_ref(&zquery);
	}

	if (!zfields) {
		MAKE_STD_ZVAL(zfields);
		array_init(zfields);
	} else {
		zval_add_ref(&zfields);
	}

	object_init_ex(return_value, mongo_ce_GridFSCursor);

	c = (mongo_collection*)zend_object_store_get_object(getThis() TSRMLS_CC);
	MONGO_CHECK_INITIALIZED(c->ns, MongoGridFS);

	MONGO_METHOD5(MongoGridFSCursor, __construct, &temp, return_value, getThis(), c->link, c->ns, zquery, zfields);

	zval_ptr_dtor(&zquery);
	zval_ptr_dtor(&zfields);
}
/* }}} */

static int get_chunk_size(zval *array TSRMLS_DC)
{
	zval **zchunk_size = 0;

	if (zend_hash_find(HASH_P(array), "chunkSize", strlen("chunkSize") + 1, (void**)&zchunk_size) == FAILURE) {
		add_assoc_long(array, "chunkSize", MonGlo(chunk_size));
		return MonGlo(chunk_size);
	}

	convert_to_long(*zchunk_size);
	return Z_LVAL_PP(zchunk_size) > 0 ?  Z_LVAL_PP(zchunk_size) : MonGlo(chunk_size);
}


static long setup_file(FILE *fp, char *filename TSRMLS_DC)
{
	long size = 0;

	/* try to open the file */
	if (!fp) {
		zend_throw_exception_ex(mongo_ce_GridFSException, 3 TSRMLS_CC, "could not open file %s", filename);
		return FAILURE;
	}

	/* get size */
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	if (size >= 0xffffffff) {
		zend_throw_exception_ex(mongo_ce_GridFSException, 4 TSRMLS_CC, "file %s is too large: %ld bytes", filename, size);
		fclose(fp);
		return FAILURE;
	}

	/* reset file ptr */
	fseek(fp, 0, SEEK_SET);

	return size;
}

static zval* setup_extra(zval *zfile, zval *extra TSRMLS_DC)
{
	zval temp;
	zval *zid = 0;
	zval **zzid = 0;

	array_init(zfile);

	/* add user-defined fields */
	if (extra) {
		zval temp;
		zend_hash_merge(HASH_P(zfile), Z_ARRVAL_P(extra), (void (*)(void*))zval_add_ref, &temp, sizeof(zval*), 1);
	}

	/* check if we need to add any fields */

	/* _id */
	if (zend_hash_find(HASH_P(zfile), "_id", strlen("_id") + 1, (void**)&zzid) == FAILURE) {
		/* create an id for the file */
		MAKE_STD_ZVAL(zid);
		object_init_ex(zid, mongo_ce_Id);
		MONGO_METHOD(MongoId, __construct, &temp, zid);

		add_assoc_zval(zfile, "_id", zid);
	} else {
		zid = *zzid;
	}
	return zid;
}

/* Use the db command to get the md5 hash of the inserted chunks
 *
 * $db->command(array(filemd5 => $fileId, "root" => $ns));
 *
 * adds the response to zfile as the "md5" field. */
static void add_md5(zval *zfile, zval *zid, mongo_collection *c TSRMLS_DC)
{
	if (!zend_hash_exists(HASH_P(zfile), "md5", strlen("md5") + 1)) {
		zval *data = 0, *response = 0, **md5 = 0;

		/* get the prefix */
		int prefix_len = strchr(Z_STRVAL_P(c->name), '.') - Z_STRVAL_P(c->name);
		char *prefix = estrndup(Z_STRVAL_P(c->name), prefix_len);

		/* create command */
		MAKE_STD_ZVAL(data);
		array_init(data);

		add_assoc_zval(data, "filemd5", zid);
		zval_add_ref(&zid);
		add_assoc_stringl(data, "root", prefix, prefix_len, 0);

		MAKE_STD_ZVAL(response);
		ZVAL_NULL(response);

		/* run command */
		MONGO_CMD(response, c->parent);

		/* make sure there wasn't an error */
		if (!EG(exception) && zend_hash_find(HASH_P(response), "md5", strlen("md5") + 1, (void**)&md5) == SUCCESS) {
			add_assoc_zval(zfile, "md5", *md5);
			/* Increment the refcount so it isn't cleaned up at the end of this
			 * method */
			zval_add_ref(md5);
		}

		/* cleanup */
		if (!EG(exception)) {
			zval_ptr_dtor(&response);
		}
		zval_ptr_dtor(&data);
	}
}

static void gridfs_rewrite_cursor_exception(TSRMLS_D)
{
	char *message = NULL;
	long code = 0;
	smart_str tmp_message = { 0 };

	if (EG(exception)) {
		message = estrdup(Z_STRVAL_P(zend_read_property(mongo_ce_GridFSException, EG(exception), "message", strlen("message"), NOISY TSRMLS_CC)));
		code = Z_LVAL_P(zend_read_property(mongo_ce_GridFSException, EG(exception), "code", strlen("code"), NOISY TSRMLS_CC));
		zend_clear_exception(TSRMLS_C);
	}

	/* create the message for the exception */
	if (message) {
		smart_str_appends(&tmp_message, "Could not store file: ");
		smart_str_appends(&tmp_message, message);
		smart_str_0(&tmp_message);
		efree(message);
	} else {
		smart_str_appends(&tmp_message, "Could not store file for unknown reasons");
		smart_str_0(&tmp_message);
	}
	zend_throw_exception(mongo_ce_GridFSException, tmp_message.c, code TSRMLS_CC);
	smart_str_free(&tmp_message);
}

static void cleanup_stale_chunks(INTERNAL_FUNCTION_PARAMETERS, zval *cleanup_ids)
{
	zval *chunks, *temp_return;
	zval **tmp;
	HashPosition pos;
	zval *tmp_exception;
	if (EG(exception)) {
		tmp_exception = EG(exception);
		EG(exception) = NULL;
	}

	chunks = zend_read_property(mongo_ce_GridFS, getThis(), "chunks", strlen("chunks"), NOISY TSRMLS_CC);

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(cleanup_ids), &pos);
	while(zend_hash_get_current_data_ex(Z_ARRVAL_P(cleanup_ids), (void **) &tmp, &pos) == SUCCESS) {
		zval *query, *cid;
		MAKE_STD_ZVAL(query);
		MAKE_STD_ZVAL(cid);
		array_init(query);
		MAKE_COPY_ZVAL(tmp, cid);

		add_assoc_zval(query, "_id", cid);

		MAKE_STD_ZVAL(temp_return);
		ZVAL_NULL(temp_return);
		MONGO_METHOD1(MongoCollection, remove, temp_return, chunks, query);

		zend_hash_move_forward_ex(Z_ARRVAL_P(cleanup_ids), &pos);
		zval_ptr_dtor(&temp_return);
		zval_ptr_dtor(&query);

	}

	if (tmp_exception) {
		EG(exception) = tmp_exception;
	}
	RETVAL_FALSE;
}

/* {{{ proto mixed MongoGridFS::storeBytes(string bytes [, array|object metadata = array() [, array options = array()]])
   Stores a string of bytes in the database */
PHP_METHOD(MongoGridFS, storeBytes)
{
	char *bytes = 0;
	int bytes_len = 0, chunk_num = 0, chunk_size = 0, global_chunk_size = 0,
	pos = 0;
	int revert = 0;

	zval temp;
	zval *extra = 0, *zid = 0, *zfile = 0, *chunks = 0, *options = 0;
	zval *cleanup_ids;
	zval *chunk_id = NULL;

	mongo_collection *c = (mongo_collection*)zend_object_store_get_object(getThis() TSRMLS_CC);
	MONGO_CHECK_INITIALIZED(c->ns, MongoGridFS);

	chunks = zend_read_property(mongo_ce_GridFS, getThis(), "chunks", strlen("chunks"), NOISY TSRMLS_CC);
	php_mongo_ensure_gridfs_index(&temp, chunks TSRMLS_CC);
	zval_dtor(&temp);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|aa/", &bytes, &bytes_len, &extra, &options) == FAILURE) {
		return;
	}

	MAKE_STD_ZVAL(cleanup_ids);
	array_init(cleanup_ids);

	/* file array object */
	MAKE_STD_ZVAL(zfile);

	/* merge extra & zfile and add _id if needed */
	zid = setup_extra(zfile, extra TSRMLS_CC);
	setup_file_fields(zfile, NULL, bytes_len TSRMLS_CC);

	/* chunkSize */
	global_chunk_size = get_chunk_size(zfile TSRMLS_CC);

	/* size */
	if (!zend_hash_exists(HASH_P(zfile), "length", strlen("length") + 1)) {
		add_assoc_long(zfile, "length", bytes_len);
	}

	/* options */
	if (!options) {
		zval *opts;
		MAKE_STD_ZVAL(opts);
		array_init(opts);
		options = opts;
	} else {
		zval_add_ref(&options);
	}

	/* insert chunks */
	while (pos < bytes_len) {
		chunk_size = bytes_len - pos >= global_chunk_size ? global_chunk_size : bytes_len - pos;

		if (!(chunk_id = insert_chunk(chunks, zid, chunk_num, bytes + pos, chunk_size, options TSRMLS_CC))) {
			revert = 1;
			goto cleanup_on_failure;
		}
		/* Keep track of that successfully inserted chunk id */
		add_next_index_zval(cleanup_ids, chunk_id);

		if (EG(exception)) {
			revert = 1;
			goto cleanup_on_failure;
		}

		/* increment counters */
		pos += chunk_size;
		chunk_num++;
	}

	/* Run GLE, just to ensure all the data has been written */
	{
		zval *data, *gle_retval;

		MAKE_STD_ZVAL(data);
		array_init(data);

		add_assoc_long(data, "getlasterror", 1);

		MAKE_STD_ZVAL(gle_retval);
		ZVAL_NULL(gle_retval);

		/* run command */
		MONGO_CMD(gle_retval, c->parent);

		if (Z_TYPE_P(gle_retval) == IS_ARRAY) {
			zval **err;

			if (zend_hash_find(Z_ARRVAL_P(gle_retval), "err", strlen("err") + 1, (void**)&err) == SUCCESS && Z_TYPE_PP(err) == IS_STRING) {
				zend_throw_exception_ex(mongo_ce_GridFSException, 0 TSRMLS_CC, Z_STRVAL_PP(err));
				/* Intentionally not returning, the exception is checked a line later */
			}
		}
		zval_ptr_dtor(&data);
		zval_ptr_dtor(&gle_retval);

		if (EG(exception)) {
			revert = 1;
			goto cleanup_on_failure;
		}
	}

	/* now that we've inserted the chunks, use them to calculate the hash */
	add_md5(zfile, zid, c TSRMLS_CC);

	/* Insert file */
	MONGO_METHOD2(MongoCollection, insert, &temp, getThis(), zfile, options);
	zval_dtor(&temp);
	if (EG(exception)) {
		revert = 1;
	}

cleanup_on_failure:
	if (revert) {
		/* Cleanup any created chunks from the chunks collection */
		/* If the insert into the files collection fails, it fails - and nothing to cleanup there anyway */
		cleanup_stale_chunks(INTERNAL_FUNCTION_PARAM_PASSTHRU, cleanup_ids);
		gridfs_rewrite_cursor_exception(TSRMLS_C);
		RETVAL_FALSE;
	} else {
		RETVAL_ZVAL(zid, 1, 0);
	}

	zval_ptr_dtor(&zfile);
	zval_ptr_dtor(&options);
	zval_ptr_dtor(&cleanup_ids);
}
/* }}} */

/* add extra fields required for files:
 * - filename
 * - upload date
 * - length
 * these fields are only added if the user hasn't defined them. */
static int setup_file_fields(zval *zfile, char *filename, int length TSRMLS_DC)
{
	zval temp;

	/* filename */
	if (filename && !zend_hash_exists(HASH_P(zfile), "filename", strlen("filename") + 1)) {
		add_assoc_stringl(zfile, "filename", filename, strlen(filename), DUP);
	}

	/* uploadDate */
	if (!zend_hash_exists(HASH_P(zfile), "uploadDate", strlen("uploadDate") + 1)) {
		zval *upload_date;
		MAKE_STD_ZVAL(upload_date);
		object_init_ex(upload_date, mongo_ce_Date);
		MONGO_METHOD(MongoDate, __construct, &temp, upload_date);

		add_assoc_zval(zfile, "uploadDate", upload_date);
	}

	/* length */
	if (!zend_hash_exists(HASH_P(zfile), "length", strlen("length") + 1)) {
		add_assoc_long(zfile, "length", length);
	}

	return SUCCESS;
}

/* Creates a chunk and adds it to the chunks collection as:
 * array(3) {
 *   files_id => zid
 *   n => chunk_num
 *   data => MongoBinData(buf, chunk_size, type 2)
 * }
 *
 * Clean up should leave:
 * - 1 ref to zid
 * - buf */
static zval* insert_chunk(zval *chunks, zval *zid, int chunk_num, char *buf, int chunk_size, zval *options  TSRMLS_DC) {
	zval temp;
	zval *zchunk, *zbin, *zretval = NULL;
	zval **_id;

	/* create chunk */
	MAKE_STD_ZVAL(zchunk);
	array_init(zchunk);

	add_assoc_zval(zchunk, "files_id", zid);
	zval_add_ref(&zid); /* zid->refcount = 2 */
	add_assoc_long(zchunk, "n", chunk_num);

	/* create MongoBinData object */
	MAKE_STD_ZVAL(zbin);
	object_init_ex(zbin, mongo_ce_BinData);
	zend_update_property_stringl(mongo_ce_BinData, zbin, "bin", strlen("bin"), buf, chunk_size TSRMLS_CC);
	zend_update_property_long(mongo_ce_BinData, zbin, "type", strlen("type"), 2 TSRMLS_CC);

	add_assoc_zval(zchunk, "data", zbin);

	/* insert chunk */
	if (options) {
		MONGO_METHOD2(MongoCollection, insert, &temp, chunks, zchunk, options);
	} else {
		MONGO_METHOD1(MongoCollection, insert, &temp, chunks, zchunk);
	}
	if (zend_hash_find(Z_ARRVAL_P(zchunk), "_id", strlen("_id") + 1, (void**)&_id) == SUCCESS) {
		MAKE_STD_ZVAL(zretval);
		ZVAL_ZVAL(zretval, *_id, 1, 0);
	}
	zval_dtor(&temp);

	/* increment counters */
	zval_ptr_dtor(&zchunk); /* zid->refcount = 1 */

	if (!zretval) {
		return NULL;
	}
	if (EG(exception)) {
		zval_ptr_dtor(&zretval);
		return NULL;
	}

	return zretval;
}

/* {{{ proto mixed MongoGridFS::storeFile(string|resource filename [, array metadata = array() [, array options = array()]])
   Stores a file in the database */
PHP_METHOD(MongoGridFS, storeFile)
{
	zval *fh, *extra = 0, *options = 0;
	char *filename = 0;
	int chunk_num = 0, global_chunk_size = 0, fd = -1;
	long size = 0, pos = 0;
	int revert = 0;
	FILE *fp = 0;

	zval temp;
	zval *zid = 0, *zfile = 0, *chunks = 0;
	zval *cleanup_ids;

	mongo_collection *c = (mongo_collection*)zend_object_store_get_object(getThis() TSRMLS_CC);
	MONGO_CHECK_INITIALIZED(c->ns, MongoGridFS);
	chunks = zend_read_property(mongo_ce_GridFS, getThis(), "chunks", strlen("chunks"), NOISY TSRMLS_CC);

	php_mongo_ensure_gridfs_index(&temp, chunks TSRMLS_CC);
	zval_dtor(&temp);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|aa/", &fh, &extra, &options) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(fh) == IS_RESOURCE) {
		zend_rsrc_list_entry *le;
		php_stdio_stream_data *stdio_fptr = 0;

		if (zend_hash_index_find(&EG(regular_list), Z_LVAL_P(fh), (void **) &le) == SUCCESS) {
			php_stream *stream = (php_stream*)le->ptr;
			if (!stream) {
				zend_throw_exception_ex(mongo_ce_GridFSException, 5 TSRMLS_CC, "could not find filehandle");
				return;
			}

			stdio_fptr = (php_stdio_stream_data*)stream->abstract;
			if (!stdio_fptr) {
				zend_throw_exception_ex(mongo_ce_GridFSException, 6 TSRMLS_CC, "no file is associate with this filehandle");
				return;
			}
		}

		if (stdio_fptr->file) {
			if ((size = setup_file(stdio_fptr->file, filename TSRMLS_CC)) == FAILURE) {
				zend_throw_exception_ex(mongo_ce_GridFSException, 7 TSRMLS_CC, "error setting up file: %s", filename);
				return;
			}
			fp = stdio_fptr->file;
		}

		fd = stdio_fptr->fd;
	} else if (Z_TYPE_P(fh) == IS_STRING) {
		filename = Z_STRVAL_P(fh);
		fp = fopen(filename, "rb");

		/* no point in continuing if we can't open the file */
		if ((size = setup_file(fp, filename TSRMLS_CC)) == FAILURE) {
			zend_throw_exception_ex(mongo_ce_GridFSException, 7 TSRMLS_CC, "error setting up file: %s", filename);
			return;
		}
	} else {
		char *msg = "first argument must be a string or stream resource";

		zend_throw_exception(zend_exception_get_default(TSRMLS_C), msg, 8 TSRMLS_CC);
		return;
	}

	/* file array object */
	MAKE_STD_ZVAL(zfile);
	ZVAL_NULL(zfile);

	/* merge extra & zfile and add _id if needed */
	zid = setup_extra(zfile, extra TSRMLS_CC);
	setup_file_fields(zfile, filename, size TSRMLS_CC);

	/* chunkSize */
	global_chunk_size = get_chunk_size(zfile TSRMLS_CC);

	/* options */
	if (!options) {
		zval *opts;
		MAKE_STD_ZVAL(opts);
		array_init(opts);
		options = opts;
	} else {
		Z_ADDREF_P(options);
	}

	MAKE_STD_ZVAL(cleanup_ids);
	array_init(cleanup_ids);

	/* insert chunks */
	while (pos < size || fp == 0) {
		int result = 0;
		char *buf;
		zval *chunk_id = NULL;

		int chunk_size = size-pos >= global_chunk_size || fp == 0 ? global_chunk_size : size-pos;
		buf = (char*)emalloc(chunk_size);

		if (fp) {
			int retval = (int)fread(buf, 1, chunk_size, fp);
			if (retval < chunk_size) {
				zend_throw_exception_ex(mongo_ce_GridFSException, 9 TSRMLS_CC, "error reading file %s", filename);
				revert = 1;
				efree(buf);
				goto cleanup_on_failure;
			}
			pos += chunk_size;
			if (!(chunk_id = insert_chunk(chunks, zid, chunk_num, buf, chunk_size, options TSRMLS_CC))) {
				revert = 1;
				efree(buf);
				goto cleanup_on_failure;
			}
			add_next_index_zval(cleanup_ids, chunk_id);
		} else {
			result = read(fd, buf, chunk_size);

			if (result == -1) {
				zend_throw_exception_ex(mongo_ce_GridFSException, 10 TSRMLS_CC, "error reading filehandle");
				revert = 1;
				efree(buf);
				goto cleanup_on_failure;
			}
			pos += result;

			if (!(chunk_id = insert_chunk(chunks, zid, chunk_num, buf, result, options TSRMLS_CC))) {
				revert = 1;
				efree(buf);
				goto cleanup_on_failure;
			}
		add_next_index_zval(cleanup_ids, chunk_id);
		}

		efree(buf);

		/* Run GLE, just to ensure all the data has been written */
		{
			zval *data, *gle_retval;

			MAKE_STD_ZVAL(data);
			array_init(data);

			add_assoc_long(data, "getlasterror", 1);

			MAKE_STD_ZVAL(gle_retval);
			ZVAL_NULL(gle_retval);

			/* run command */
			MONGO_CMD(gle_retval, c->parent);

			if (Z_TYPE_P(gle_retval) == IS_ARRAY) {
				zval **err;

				if (zend_hash_find(Z_ARRVAL_P(gle_retval), "err", strlen("err") + 1, (void**)&err) == SUCCESS && Z_TYPE_PP(err) == IS_STRING) {
					zend_throw_exception_ex(mongo_ce_GridFSException, 0 TSRMLS_CC, Z_STRVAL_PP(err));
					/* Intentionally not returning, the exception is checked a line later */
				}
			}
			zval_ptr_dtor(&data);
			zval_ptr_dtor(&gle_retval);
			if (EG(exception)) {
				revert = 1;
				goto cleanup_on_failure;
			}
		}

		if (EG(exception)) {
			revert = 1;
			goto cleanup_on_failure;
		}

		chunk_num++;

		if (fp == 0 && result < chunk_size) {
			break;
		}
	}

	/* close file ptr */
	if (fp) {
		fclose(fp);
	}

	if (EG(exception)) {
		goto cleanup_on_failure;
	}

	if (!fp) {
		add_assoc_long(zfile, "length", pos);
	}

	add_md5(zfile, zid, c TSRMLS_CC);

	/* insert file */
	if (!revert) {
		zval *temp_return;

		Z_ADDREF_P(options);
		MAKE_STD_ZVAL(temp_return);
		ZVAL_NULL(temp_return);
		MONGO_METHOD2(MongoCollection, insert, temp_return, getThis(), zfile, options);
		zval_ptr_dtor(&temp_return);
		Z_DELREF_P(options);

		if (EG(exception)) {
			revert = 1;
		}
	}

	if (!revert) {
		RETVAL_ZVAL(zid, 1, 0);
	}

cleanup_on_failure:
	/* remove all inserted chunks and main file document */
	if (revert) {
		/* Cleanup any created chunks from the chunks collection. If the insert
		 * into the files collection fails, it fails - and nothing to cleanup
		 * there anyway */
		cleanup_stale_chunks(INTERNAL_FUNCTION_PARAM_PASSTHRU, cleanup_ids);
		gridfs_rewrite_cursor_exception(TSRMLS_C);
		RETVAL_FALSE;
	}

	/* cleanup */
	zval_ptr_dtor(&zfile);
	zval_ptr_dtor(&cleanup_ids);

	zval_ptr_dtor(&options);
}
/* }}} */

/* {{{ proto MongoGridFSFile MongoGridFS::findOne([array|string query = array() [, array|object fields = array()]])
   Returns a single file matching the criteria. If $query is a string, it will
   be used to match documents by the filename field, which may not be unique. */
PHP_METHOD(MongoGridFS, findOne)
{
	zval *zquery = 0, *zfields = 0, *file;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zz", &zquery, &zfields) == FAILURE) {
		return;
	}
	MUST_BE_ARRAY_OR_OBJECT(2, zfields);

	if (!zquery) {
		MAKE_STD_ZVAL(zquery);
		array_init(zquery);
	} else if (Z_TYPE_P(zquery) != IS_ARRAY) {
		zval *temp;

		convert_to_string(zquery);

		MAKE_STD_ZVAL(temp);
		array_init(temp);
		add_assoc_string(temp, "filename", Z_STRVAL_P(zquery), 1);

		zquery = temp;
	} else {
		zval_add_ref(&zquery);
	}

	if (!zfields) {
		MAKE_STD_ZVAL(zfields);
		array_init(zfields);
	} else {
		zval_add_ref(&zfields);
	}

	MAKE_STD_ZVAL(file);
	MONGO_METHOD2(MongoCollection, findOne, file, getThis(), zquery, zfields);

	if (Z_TYPE_P(file) == IS_NULL) {
		RETVAL_NULL();
	} else {
		zval temp;

		object_init_ex(return_value, mongo_ce_GridFSFile);
		MONGO_METHOD2(MongoGridFSFile, __construct, &temp, return_value, getThis(), file);
	}

	zval_ptr_dtor(&file);
	zval_ptr_dtor(&zquery);
	zval_ptr_dtor(&zfields);
}
/* }}} */

/* {{{ proto mixed MongoGridFS::remove([array|string criteria = array() [, array options = array()]])
   Removes files and corresponding chunks. If $query is a string, it will be
   used to match documents by the filename field, which may not be unique. */
PHP_METHOD(MongoGridFS, remove)
{
	zval *criteria = 0, *options = 0, *zfields, *zcursor, *chunks, *next, temp;
	zval **tmp;
	int justOne = -1;

	chunks = zend_read_property(mongo_ce_GridFS, getThis(), "chunks", strlen("chunks"), NOISY TSRMLS_CC);
	php_mongo_ensure_gridfs_index(&temp, chunks TSRMLS_CC);
	zval_dtor(&temp);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|za/", &criteria, &options) == FAILURE) {
		return;
	}

	if (!criteria) {
		MAKE_STD_ZVAL(criteria);
		array_init(criteria);
	} else if (Z_TYPE_P(criteria) == IS_ARRAY) {
		zval_add_ref(&criteria);
	} else {
		zval *tmp;

		MAKE_STD_ZVAL(tmp);
		array_init(tmp);
		convert_to_string(criteria);
		add_assoc_stringl(tmp, "filename", Z_STRVAL_P(criteria), Z_STRLEN_P(criteria), 1);
		criteria = tmp;
	}

	if (!options) {
		MAKE_STD_ZVAL(options);
		array_init(options);
	}

	/* { _id : 1 } */
	MAKE_STD_ZVAL(zfields);
	array_init(zfields);
	add_assoc_long(zfields, "_id", 1);

	/* cursor = db.fs.files.find(criteria, {_id : 1}); */
	MAKE_STD_ZVAL(zcursor);
	MONGO_METHOD2(MongoCollection, find, zcursor, getThis(), criteria, zfields);
	zval_ptr_dtor(&zfields);
	PHP_MONGO_CHECK_EXCEPTION3(&zcursor, &criteria, &options);

	MAKE_STD_ZVAL(next);
	MONGO_METHOD(MongoCursor, getNext, next, zcursor);
	PHP_MONGO_CHECK_EXCEPTION4(&next, &zcursor, &criteria, &options);

	/* Temporarily ignore the justOne option while cleaning the data by _id */
	if (zend_hash_find(Z_ARRVAL_P(options), "justOne", strlen("justOne") + 1, (void**)&tmp) == SUCCESS) {
		convert_to_boolean(*tmp);
		justOne = Z_BVAL_PP(tmp);
		add_assoc_bool(options, "justOne", 0);
	}

	while (Z_TYPE_P(next) != IS_NULL) {
		zval **id;
		zval *temp, *temp_return;

		if (zend_hash_find(HASH_P(next), "_id", 4, (void**)&id) == FAILURE) {
			/* uh oh */
			continue;
		}

		MAKE_STD_ZVAL(temp);
		array_init(temp);
		zval_add_ref(id);
		add_assoc_zval(temp, "files_id", *id);

		MAKE_STD_ZVAL(temp_return);
		ZVAL_NULL(temp_return);

		MONGO_METHOD2(MongoCollection, remove, temp_return, chunks, temp, options);
		zval_ptr_dtor(&temp);
		zval_ptr_dtor(&temp_return);
		zval_ptr_dtor(&next);
		PHP_MONGO_CHECK_EXCEPTION3(&zcursor, &criteria, &options);

		MAKE_STD_ZVAL(next);
		MONGO_METHOD(MongoCursor, getNext, next, zcursor);
		PHP_MONGO_CHECK_EXCEPTION4(&next, &zcursor, &criteria, &options);
	}
	zval_ptr_dtor(&next);
	zval_ptr_dtor(&zcursor);

	/* Restore the justOne option when we cleanup the files collection itself */
	if (justOne != -1) {
		add_assoc_bool(options, "justOne", justOne);
	}
	MONGO_METHOD2(MongoCollection, remove, return_value, getThis(), criteria, options);

	zval_ptr_dtor(&criteria);
	zval_ptr_dtor(&options);
}
/* }}} */

/* {{{ proto mixed MongoGridFS::storeUpload(string name [, array|string metadata = array()])
   Stores an uploaded file in the database. If $metadata is a string, it will be
   used as the filename field for the stored file. */
PHP_METHOD(MongoGridFS, storeUpload)
{
	zval *extra = 0, **file, **temp, **name = 0, *extra_param = 0;
	const zval *h;
	char *filename = 0;
	int file_len = 0, found_name = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &filename, &file_len, &extra) == FAILURE) {
		return;
	}

	h = PG(http_globals)[TRACK_VARS_FILES];
	if (zend_hash_find(Z_ARRVAL_P(h), filename, file_len + 1, (void**)&file) == FAILURE || Z_TYPE_PP(file) != IS_ARRAY) {
		zend_throw_exception_ex(mongo_ce_GridFSException, 11 TSRMLS_CC, "could not find uploaded file %s", filename);
		return;
	}

	if (extra && Z_TYPE_P(extra) == IS_ARRAY) {
		zval_add_ref(&extra);
		extra_param = extra;

		if (zend_hash_exists(HASH_P(extra), "filename", strlen("filename") + 1)) {
			found_name = 1;
		}
	} else {
		MAKE_STD_ZVAL(extra_param);
		array_init(extra_param);

		if (extra && Z_TYPE_P(extra) == IS_STRING) {
			add_assoc_string(extra_param, "filename", Z_STRVAL_P(extra), 1);
			found_name = 1;
		}
	}

	zend_hash_find(Z_ARRVAL_PP(file), "tmp_name", strlen("tmp_name") + 1, (void**)&temp);
	if (!temp) {
		zend_throw_exception(mongo_ce_GridFSException, "Couldn't find tmp_name in the $_FILES array. Are you sure the upload worked?", 12 TSRMLS_CC);
		return;
	}

	if (Z_TYPE_PP(temp) == IS_ARRAY) {
		HashPosition pos;
		zval **entry, **names;

		zend_hash_find(Z_ARRVAL_PP(file), "name", strlen("name") + 1, (void**)&names);

		array_init(return_value);
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(names));
		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_PP(temp), &pos);

		while (zend_hash_get_current_data_ex(Z_ARRVAL_PP(temp), (void **)&entry, &pos) == SUCCESS) {
			zval *retval;
			zval *copied;
			zval **name;
			MAKE_STD_ZVAL(retval);

			zend_hash_get_current_data(Z_ARRVAL_PP(names), (void **)&name);
			add_assoc_string(extra_param, "filename", Z_STRVAL_PP(name), 1);

			MONGO_METHOD2(MongoGridFS, storeFile, retval, getThis(), *entry, extra_param);

			ALLOC_ZVAL(copied);
			MAKE_COPY_ZVAL(&retval, copied);
			Z_ADDREF_P(copied);

			zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &copied, sizeof(zval *), NULL);
			zend_hash_move_forward_ex(Z_ARRVAL_PP(temp), &pos);
			zend_hash_move_forward(Z_ARRVAL_PP(names));

			zval_ptr_dtor(&retval);
			zval_ptr_dtor(&copied);
		}

		zval_ptr_dtor(&extra_param);
	} else if (Z_TYPE_PP(temp) == IS_STRING) {
		if (
			!found_name &&
			zend_hash_find(Z_ARRVAL_PP(file), "name", strlen("name") + 1, (void**)&name) == SUCCESS &&
			Z_TYPE_PP(name) == IS_STRING
		) {
			add_assoc_string(extra_param, "filename", Z_STRVAL_PP(name), 1);
		}

		MONGO_METHOD2(MongoGridFS, storeFile, return_value, getThis(), *temp, extra_param);
		zval_ptr_dtor(&extra_param);
	} else {
		zend_throw_exception(mongo_ce_GridFSException, "tmp_name was not a string or an array", 13 TSRMLS_CC);
	}
}
/* }}} */

/*
 * New GridFS API
 */

/* {{{ proto bool MongoGridFS::delete(mixed id)
   Delete a file from the database */
PHP_METHOD(MongoGridFS, delete)
{
	zval *id, *criteria;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &id) == FAILURE) {
		return;
	}

	/* Set up criteria array */
	MAKE_STD_ZVAL(criteria);
	array_init(criteria);
	add_assoc_zval(criteria, "_id", id);
	zval_add_ref(&id);

	MONGO_METHOD1(MongoGridFS, remove, return_value, getThis(), criteria);

	zval_ptr_dtor(&criteria);
}
/* }}} */

/* {{{ proto MongoGridFSFile MongoGridFS::get(mixed id)
   Retrieve a file from the database */
PHP_METHOD(MongoGridFS, get)
{
	zval *id, *criteria;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &id) == FAILURE) {
		return;
	}

	MAKE_STD_ZVAL(criteria);
	array_init(criteria);
	add_assoc_zval(criteria, "_id", id);
	zval_add_ref(&id);

	MONGO_METHOD1(MongoGridFS, findOne, return_value, getThis(), criteria);

	zval_ptr_dtor(&criteria);
}
/* }}} */

/* {{{ proto mixed MongoGridFS::put(string|resource filename [, array metadata = array() [, array options = array()]])
   Stores a file in the database */
PHP_METHOD(MongoGridFS, put)
{
	MONGO_METHOD_BASE(MongoGridFS, storeFile)(ZEND_NUM_ARGS(), return_value, NULL, getThis(), 0 TSRMLS_CC);
}
/* }}} */

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_find, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, fields)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_find_one, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, fields)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_remove, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, filename_OR_fields_OR_object)
	ZEND_ARG_ARRAY_INFO(0, options, 0)
ZEND_END_ARG_INFO()


static zend_function_entry MongoGridFS_methods[] = {
	PHP_ME(MongoGridFS, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFS, drop, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFS, find, arginfo_find, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFS, storeFile, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFS, storeBytes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFS, findOne, arginfo_find_one, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFS, remove, arginfo_remove, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFS, storeUpload, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFS, delete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFS, get, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFS, put, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

void mongo_init_MongoGridFS(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoGridFS", MongoGridFS_methods);
	ce.create_object = php_mongo_collection_new;
	mongo_ce_GridFS = zend_register_internal_class_ex(&ce, mongo_ce_Collection, "MongoCollection" TSRMLS_CC);

	zend_declare_property_null(mongo_ce_GridFS, "chunks", strlen("chunks"), ZEND_ACC_PUBLIC TSRMLS_CC);

	zend_declare_property_null(mongo_ce_GridFS, "filesName", strlen("filesName"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(mongo_ce_GridFS, "chunksName", strlen("chunksName"), ZEND_ACC_PROTECTED TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
