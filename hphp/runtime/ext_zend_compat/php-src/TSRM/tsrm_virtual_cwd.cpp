#include "hphp/runtime/ext_zend_compat/php-src/TSRM/tsrm_virtual_cwd.h"

#include "hphp/runtime/ext/ext_file.h"

#define CWD_STATE_COPY(d, s)				\
	(d)->cwd_length = (s)->cwd_length;		\
	(d)->cwd = (char *) malloc((s)->cwd_length+1);	\
	memcpy((d)->cwd, (s)->cwd, (s)->cwd_length+1);

#define CWD_STATE_FREE(s)			\
	free((s)->cwd);

#ifdef ZTS
ts_rsrc_id cwd_globals_id;
#else
virtual_cwd_globals cwd_globals;
#endif

CWD_API char *tsrm_realpath(const char *path, char *real_path TSRMLS_DC) {
  HPHP::Variant rp = HPHP::f_realpath(path);
  if (rp.isBoolean()) {
    assert(!rp.toBoolean());
    return nullptr;
  }

  HPHP::StringData *ret = rp.getStringData();

  if (real_path) {
    int copy_len = ret->size();
    memcpy(real_path, ret->data(), copy_len);
    real_path[copy_len] = '\0';
    return real_path;
  } else {
    return strndup(ret->data(), ret->size());
  }
}

#ifdef PHP_WIN32
static inline unsigned long realpath_cache_key(const char *path, int path_len TSRMLS_DC) /* {{{ */
{
	register unsigned long h;
	char *bucket_key_start = tsrm_win32_get_path_sid_key(path TSRMLS_CC);
	char *bucket_key = (char *)bucket_key_start;
	const char *e = bucket_key + strlen(bucket_key);

	if (!bucket_key) {
		return 0;
	}

	for (h = 2166136261U; bucket_key < e;) {
		h *= 16777619;
		h ^= *bucket_key++;
	}
	HeapFree(GetProcessHeap(), 0, (LPVOID)bucket_key_start);
	return h;
}
/* }}} */
#else
static inline unsigned long realpath_cache_key(const char *path, int path_len) /* {{{ */
{
	register unsigned long h;
	const char *e = path + path_len;

	for (h = 2166136261U; path < e;) {
		h *= 16777619;
		h ^= *path++;
	}

	return h;
}
/* }}} */
#endif /* defined(PHP_WIN32) */


static inline void realpath_cache_add(const char *path, int path_len, const char *realpath, int realpath_len, int is_dir, time_t t TSRMLS_DC) /* {{{ */
{
	long size = sizeof(realpath_cache_bucket) + path_len + 1;
	int same = 1;

	if (realpath_len != path_len ||
		memcmp(path, realpath, path_len) != 0) {
		size += realpath_len + 1;
		same = 0;
	}

	if (CWDG(realpath_cache_size) + size <= CWDG(realpath_cache_size_limit)) {
		realpath_cache_bucket *bucket = (realpath_cache_bucket*) malloc(size);
		unsigned long n;

		if (bucket == NULL) {
			return;
		}

#ifdef PHP_WIN32
		bucket->key = realpath_cache_key(path, path_len TSRMLS_CC);
#else
		bucket->key = realpath_cache_key(path, path_len);
#endif
		bucket->path = (char*)bucket + sizeof(realpath_cache_bucket);
		memcpy(bucket->path, path, path_len+1);
		bucket->path_len = path_len;
		if (same) {
			bucket->realpath = bucket->path;
		} else {
			bucket->realpath = bucket->path + (path_len + 1);
			memcpy(bucket->realpath, realpath, realpath_len+1);
		}
		bucket->realpath_len = realpath_len;
		bucket->is_dir = is_dir;
#ifdef PHP_WIN32
		bucket->is_rvalid   = 0;
		bucket->is_readable = 0;
		bucket->is_wvalid   = 0;
		bucket->is_writable = 0;
#endif
		bucket->expires = t + CWDG(realpath_cache_ttl);
		n = bucket->key % (sizeof(CWDG(realpath_cache)) / sizeof(CWDG(realpath_cache)[0]));
		bucket->next = CWDG(realpath_cache)[n];
		CWDG(realpath_cache)[n] = bucket;
		CWDG(realpath_cache_size) += size;
	}
}
/* }}} */


static inline realpath_cache_bucket* realpath_cache_find(const char *path, int path_len, time_t t TSRMLS_DC) /* {{{ */
{
#ifdef PHP_WIN32
	unsigned long key = realpath_cache_key(path, path_len TSRMLS_CC);
#else
	unsigned long key = realpath_cache_key(path, path_len);
#endif

	unsigned long n = key % (sizeof(CWDG(realpath_cache)) / sizeof(CWDG(realpath_cache)[0]));
	realpath_cache_bucket **bucket = &CWDG(realpath_cache)[n];

	while (*bucket != NULL) {
		if (CWDG(realpath_cache_ttl) && (*bucket)->expires < t) {
			realpath_cache_bucket *r = *bucket;
			*bucket = (*bucket)->next;

			/* if the pointers match then only subtract the length of the path */		
		   	if(r->path == r->realpath) {
				CWDG(realpath_cache_size) -= sizeof(realpath_cache_bucket) + r->path_len + 1;
			} else {
				CWDG(realpath_cache_size) -= sizeof(realpath_cache_bucket) + r->path_len + 1 + r->realpath_len + 1;
			}
			free(r);
		} else if (key == (*bucket)->key && path_len == (*bucket)->path_len &&
					memcmp(path, (*bucket)->path, path_len) == 0) {
			return *bucket;
		} else {
			bucket = &(*bucket)->next;
		}
	}
	return NULL;
}
/* }}} */

CWD_API realpath_cache_bucket* realpath_cache_lookup(const char *path, int path_len, time_t t TSRMLS_DC) /* {{{ */
{
	return realpath_cache_find(path, path_len, t TSRMLS_CC);
}
/* }}} */

CWD_API int realpath_cache_size(TSRMLS_D)
{
	return CWDG(realpath_cache_size);
}

CWD_API int realpath_cache_max_buckets(TSRMLS_D)
{
	return (sizeof(CWDG(realpath_cache)) / sizeof(CWDG(realpath_cache)[0]));
}

CWD_API realpath_cache_bucket** realpath_cache_get_buckets(TSRMLS_D)
{
	return CWDG(realpath_cache);
}


#undef LINK_MAX
#define LINK_MAX 32

static int tsrm_realpath_r(char *path, int start, int len, int *ll, time_t *t, int use_realpath, int is_dir, int *link_is_dir TSRMLS_DC) /* {{{ */
{
	int i, j, save;
	int directory = 0;
#ifdef TSRM_WIN32
	WIN32_FIND_DATA data;
	HANDLE hFind;
	TSRM_ALLOCA_FLAG(use_heap_large)
#else
	struct stat st;
#endif
	realpath_cache_bucket *bucket;
	char *tmp;
	TSRM_ALLOCA_FLAG(use_heap)

	while (1) {
		if (len <= start) {
			if (link_is_dir) {
				*link_is_dir = 1;
			}
			return start;
		}

		i = len;
		while (i > start && !IS_SLASH(path[i-1])) {
			i--;
		}

		if (i == len ||
			(i == len - 1 && path[i] == '.')) {
			/* remove double slashes and '.' */
			len = i - 1;
			is_dir = 1;
			continue;
		} else if (i == len - 2 && path[i] == '.' && path[i+1] == '.') {
			/* remove '..' and previous directory */
			is_dir = 1;
			if (link_is_dir) {
				*link_is_dir = 1;
			}
			if (i - 1 <= start) {
				return start ? start : len;
			}
			j = tsrm_realpath_r(path, start, i-1, ll, t, use_realpath, 1, NULL TSRMLS_CC);
			if (j > start) {
				j--;
				while (j > start && !IS_SLASH(path[j])) {
					j--;
				}
				if (!start) {
					/* leading '..' must not be removed in case of relative path */
					if (j == 0 && path[0] == '.' && path[1] == '.' &&
							IS_SLASH(path[2])) {
						path[3] = '.';
						path[4] = '.';
						path[5] = DEFAULT_SLASH;
						j = 5;
					} else if (j > 0 &&
							path[j+1] == '.' && path[j+2] == '.' &&
							IS_SLASH(path[j+3])) {
						j += 4;
						path[j++] = '.';
						path[j++] = '.';
						path[j] = DEFAULT_SLASH;
					}
				}
			} else if (!start && !j) {
				/* leading '..' must not be removed in case of relative path */
				path[0] = '.';
				path[1] = '.';
				path[2] = DEFAULT_SLASH;
				j = 2;
			}
			return j;
		}

		path[len] = 0;

		save = (use_realpath != CWD_EXPAND);

		if (start && save && CWDG(realpath_cache_size_limit)) {
			/* cache lookup for absolute path */
			if (!*t) {
				*t = time(0);
			}
			if ((bucket = realpath_cache_find(path, len, *t TSRMLS_CC)) != NULL) {
				if (is_dir && !bucket->is_dir) {
					/* not a directory */
					return -1;
				} else {
					if (link_is_dir) {
						*link_is_dir = bucket->is_dir;
					}
					memcpy(path, bucket->realpath, bucket->realpath_len + 1);
					return bucket->realpath_len;
				}
			}
		}

#ifdef TSRM_WIN32
		if (save && (hFind = FindFirstFile(path, &data)) == INVALID_HANDLE_VALUE) {
			if (use_realpath == CWD_REALPATH) {
				/* file not found */
				return -1;
			}
			/* continue resolution anyway but don't save result in the cache */
			save = 0;
		}

		if (save) {
			FindClose(hFind);
		}

		tmp = tsrm_do_alloca(len+1, use_heap);
		memcpy(tmp, path, len+1);

		if(save &&
				!(IS_UNC_PATH(path, len) && len >= 3 && path[2] != '?') &&
				(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
			/* File is a reparse point. Get the target */
			HANDLE hLink = NULL;
			REPARSE_DATA_BUFFER * pbuffer;
			unsigned int retlength = 0;
			int bufindex = 0, isabsolute = 0;
			wchar_t * reparsetarget;
			BOOL isVolume = FALSE;
			char printname[MAX_PATH];
			char substitutename[MAX_PATH];
			int printname_len, substitutename_len;
			int substitutename_off = 0;

			if(++(*ll) > LINK_MAX) {
				return -1;
			}

			hLink = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS, NULL);
			if(hLink == INVALID_HANDLE_VALUE) {
				return -1;
			}

			pbuffer = (REPARSE_DATA_BUFFER *)tsrm_do_alloca(MAXIMUM_REPARSE_DATA_BUFFER_SIZE, use_heap_large);
			if (pbuffer == NULL) {
				return -1;
			}
			if(!DeviceIoControl(hLink, FSCTL_GET_REPARSE_POINT, NULL, 0, pbuffer,  MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &retlength, NULL)) {
				tsrm_free_alloca(pbuffer, use_heap_large);
				CloseHandle(hLink);
				return -1;
			}

			CloseHandle(hLink);

			if(pbuffer->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
				reparsetarget = pbuffer->SymbolicLinkReparseBuffer.ReparseTarget;
				printname_len = pbuffer->MountPointReparseBuffer.PrintNameLength / sizeof(WCHAR);
				isabsolute = (pbuffer->SymbolicLinkReparseBuffer.Flags == 0) ? 1 : 0;
				if (!WideCharToMultiByte(CP_THREAD_ACP, 0,
					reparsetarget + pbuffer->MountPointReparseBuffer.PrintNameOffset  / sizeof(WCHAR),
					printname_len + 1,
					printname, MAX_PATH, NULL, NULL
				)) {
					tsrm_free_alloca(pbuffer, use_heap_large);
					return -1;
				};
				printname_len = pbuffer->MountPointReparseBuffer.PrintNameLength / sizeof(WCHAR);
				printname[printname_len] = 0;

				substitutename_len = pbuffer->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
				if (!WideCharToMultiByte(CP_THREAD_ACP, 0,
					reparsetarget + pbuffer->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR),
					substitutename_len + 1,
					substitutename, MAX_PATH, NULL, NULL
				)) {
					tsrm_free_alloca(pbuffer, use_heap_large);
					return -1;
				};
				substitutename[substitutename_len] = 0;
			}
			else if(pbuffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
				isabsolute = 1;
				reparsetarget = pbuffer->MountPointReparseBuffer.ReparseTarget;
				printname_len = pbuffer->MountPointReparseBuffer.PrintNameLength / sizeof(WCHAR);
				if (!WideCharToMultiByte(CP_THREAD_ACP, 0,
					reparsetarget + pbuffer->MountPointReparseBuffer.PrintNameOffset  / sizeof(WCHAR),
					printname_len + 1,
					printname, MAX_PATH, NULL, NULL
				)) {
					tsrm_free_alloca(pbuffer, use_heap_large);
					return -1;
				};
				printname[pbuffer->MountPointReparseBuffer.PrintNameLength / sizeof(WCHAR)] = 0;

				substitutename_len = pbuffer->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
				if (!WideCharToMultiByte(CP_THREAD_ACP, 0,
					reparsetarget + pbuffer->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR),
					substitutename_len + 1,
					substitutename, MAX_PATH, NULL, NULL
				)) {
					tsrm_free_alloca(pbuffer, use_heap_large);
					return -1;
				};
				substitutename[substitutename_len] = 0;
			}
			else if (pbuffer->ReparseTag == IO_REPARSE_TAG_DEDUP) {
				isabsolute = 1;
				memcpy(substitutename, path, len + 1);
				substitutename_len = len;
			} else {
				tsrm_free_alloca(pbuffer, use_heap_large);
				return -1;
			}

			if(isabsolute && substitutename_len > 4) {
				/* Do not resolve volumes (for now). A mounted point can
				   target a volume without a drive, it is not certain that
				   all IO functions we use in php and its deps support
				   path with volume GUID instead of the DOS way, like:
				   d:\test\mnt\foo
				   \\?\Volume{62d1c3f8-83b9-11de-b108-806e6f6e6963}\foo
				*/
				if (strncmp(substitutename, "\\??\\Volume{",11) == 0
					|| strncmp(substitutename, "\\\\?\\Volume{",11) == 0
					|| strncmp(substitutename, "\\??\\UNC\\", 8) == 0
					) {
					isVolume = TRUE;
					substitutename_off = 0;
				} else
					/* do not use the \??\ and \\?\ prefix*/
					if (strncmp(substitutename, "\\??\\", 4) == 0
						|| strncmp(substitutename, "\\\\?\\", 4) == 0) {
					substitutename_off = 4;
				}
			}

			if (!isVolume) {
				char * tmp2 = substitutename + substitutename_off;
				for(bufindex = 0; bufindex < (substitutename_len - substitutename_off); bufindex++) {
					*(path + bufindex) = *(tmp2 + bufindex);
				}

				*(path + bufindex) = 0;
				j = bufindex;
			} else {
				j = len;
			}


#if VIRTUAL_CWD_DEBUG
			fprintf(stderr, "reparse: print: %s ", printname);
			fprintf(stderr, "sub: %s ", substitutename);
			fprintf(stderr, "resolved: %s ", path);
#endif
			tsrm_free_alloca(pbuffer, use_heap_large);

			if(isabsolute == 1) {
				if (!((j == 3) && (path[1] == ':') && (path[2] == '\\'))) {
					/* use_realpath is 0 in the call below coz path is absolute*/
					j = tsrm_realpath_r(path, 0, j, ll, t, 0, is_dir, &directory TSRMLS_CC);
					if(j < 0) {
						tsrm_free_alloca(tmp, use_heap);
						return -1;
					}
				}
			}
			else {
				if(i + j >= MAXPATHLEN - 1) {
					tsrm_free_alloca(tmp, use_heap);
					return -1;
				}

				memmove(path+i, path, j+1);
				memcpy(path, tmp, i-1);
				path[i-1] = DEFAULT_SLASH;
				j  = tsrm_realpath_r(path, start, i + j, ll, t, use_realpath, is_dir, &directory TSRMLS_CC);
				if(j < 0) {
					tsrm_free_alloca(tmp, use_heap);
					return -1;
				}
			}
			directory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

			if(link_is_dir) {
				*link_is_dir = directory;
			}
		}
		else {
			if (save) {
				directory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
				if (is_dir && !directory) {
					/* not a directory */
					return -1;
				}
			}

#elif defined(NETWARE)
		save = 0;
		tmp = tsrm_do_alloca(len+1, use_heap);
		memcpy(tmp, path, len+1);
#else
		if (save && php_sys_lstat(path, &st) < 0) {
			if (use_realpath == CWD_REALPATH) {
				/* file not found */
				return -1;
			}
			/* continue resolution anyway but don't save result in the cache */
			save = 0;
		}

		tmp = (char*) tsrm_do_alloca(len+1, use_heap);
		memcpy(tmp, path, len+1);

		if (save && S_ISLNK(st.st_mode)) {
			if (++(*ll) > LINK_MAX || (j = php_sys_readlink(tmp, path, MAXPATHLEN)) < 0) {
				/* too many links or broken symlinks */
				tsrm_free_alloca(tmp, use_heap);
				return -1;
			}
			path[j] = 0;
			if (IS_ABSOLUTE_PATH(path, j)) {
				j = tsrm_realpath_r(path, 1, j, ll, t, use_realpath, is_dir, &directory TSRMLS_CC);
				if (j < 0) {
					tsrm_free_alloca(tmp, use_heap);
					return -1;
				}
			} else {
				if (i + j >= MAXPATHLEN-1) {
					tsrm_free_alloca(tmp, use_heap);
					return -1; /* buffer overflow */
				}
				memmove(path+i, path, j+1);
				memcpy(path, tmp, i-1);
				path[i-1] = DEFAULT_SLASH;
				j = tsrm_realpath_r(path, start, i + j, ll, t, use_realpath, is_dir, &directory TSRMLS_CC);
				if (j < 0) {
					tsrm_free_alloca(tmp, use_heap);
					return -1;
				}
			}
			if (link_is_dir) {
				*link_is_dir = directory;
			}
		} else {
			if (save) {
				directory = S_ISDIR(st.st_mode);
				if (link_is_dir) {
					*link_is_dir = directory;
				}
				if (is_dir && !directory) {
					/* not a directory */
					tsrm_free_alloca(tmp, use_heap);
					return -1;
				}
			}
#endif
			if (i - 1 <= start) {
				j = start;
			} else {
				/* some leading directories may be unaccessable */
				j = tsrm_realpath_r(path, start, i-1, ll, t, save ? CWD_FILEPATH : use_realpath, 1, NULL TSRMLS_CC);
				if (j > start) {
					path[j++] = DEFAULT_SLASH;
				}
			}
#ifdef TSRM_WIN32
			if (j < 0 || j + len - i >= MAXPATHLEN-1) {
				tsrm_free_alloca(tmp, use_heap);
				return -1;
			}
			if (save) {
				i = strlen(data.cFileName);
				memcpy(path+j, data.cFileName, i+1);
				j += i;
			} else {
				/* use the original file or directory name as it wasn't found */
				memcpy(path+j, tmp+i, len-i+1);
				j += (len-i);
			}
		}
#else
			if (j < 0 || j + len - i >= MAXPATHLEN-1) {
				tsrm_free_alloca(tmp, use_heap);
				return -1;
			}
			memcpy(path+j, tmp+i, len-i+1);
			j += (len-i);
		}
#endif

		if (save && start && CWDG(realpath_cache_size_limit)) {
			/* save absolute path in the cache */
			realpath_cache_add(tmp, len, path, j, directory, *t TSRMLS_CC);
		}

		tsrm_free_alloca(tmp, use_heap);
		return j;
	}
}
/* }}} */

/* Resolve path relatively to state and put the real path into state */
/* returns 0 for ok, 1 for error */
CWD_API int virtual_file_ex(cwd_state *state, const char *path, verify_path_func verify_path, int use_realpath TSRMLS_DC) /* {{{ */
{
	int path_length = strlen(path);
	char resolved_path[MAXPATHLEN];
	int start = 1;
	int ll = 0;
	time_t t;
	int ret;
	int add_slash;
	void *tmp;

	if (path_length == 0 || path_length >= MAXPATHLEN-1) {
#ifdef TSRM_WIN32
# if _MSC_VER < 1300
		errno = EINVAL;
# else
		_set_errno(EINVAL);
# endif
#else
		errno = EINVAL;
#endif
		return 1;
	}

#if VIRTUAL_CWD_DEBUG
	fprintf(stderr,"cwd = %s path = %s\n", state->cwd, path);
#endif

	/* cwd_length can be 0 when getcwd() fails.
	 * This can happen under solaris when a dir does not have read permissions
	 * but *does* have execute permissions */
	if (!IS_ABSOLUTE_PATH(path, path_length)) {
		if (state->cwd_length == 0) {
			/* resolve relative path */
			start = 0;
			memcpy(resolved_path , path, path_length + 1);
		} else {
			int state_cwd_length = state->cwd_length;

#ifdef TSRM_WIN32
			if (IS_SLASH(path[0])) {
				if (state->cwd[1] == ':') {
					/* Copy only the drive name */
					state_cwd_length = 2;
				} else if (IS_UNC_PATH(state->cwd, state->cwd_length)) {
					/* Copy only the share name */
					state_cwd_length = 2;
					while (IS_SLASH(state->cwd[state_cwd_length])) {
						state_cwd_length++;
					}
					while (state->cwd[state_cwd_length] &&
							!IS_SLASH(state->cwd[state_cwd_length])) {
						state_cwd_length++;
					}
					while (IS_SLASH(state->cwd[state_cwd_length])) {
						state_cwd_length++;
					}
					while (state->cwd[state_cwd_length] &&
							!IS_SLASH(state->cwd[state_cwd_length])) {
						state_cwd_length++;
					}
				}
			}
#endif
			if (path_length + state_cwd_length + 1 >= MAXPATHLEN-1) {
				return 1;
			}
			memcpy(resolved_path, state->cwd, state_cwd_length);
			if (resolved_path[state_cwd_length-1] == DEFAULT_SLASH) {
				memcpy(resolved_path + state_cwd_length, path, path_length + 1);
				path_length += state_cwd_length;
			} else {
				resolved_path[state_cwd_length] = DEFAULT_SLASH;
				memcpy(resolved_path + state_cwd_length + 1, path, path_length + 1);
				path_length += state_cwd_length + 1;
			}
		}
	} else {
#ifdef TSRM_WIN32
		if (path_length > 2 && path[1] == ':' && !IS_SLASH(path[2])) {
			resolved_path[0] = path[0];
			resolved_path[1] = ':';
			resolved_path[2] = DEFAULT_SLASH;
			memcpy(resolved_path + 3, path + 2, path_length - 1);
			path_length++;
		} else
#endif
		memcpy(resolved_path, path, path_length + 1);
	}

#ifdef TSRM_WIN32
	if (memchr(resolved_path, '*', path_length) ||
		memchr(resolved_path, '?', path_length)) {
		return 1;
	}
#endif

#ifdef TSRM_WIN32
	if (IS_UNC_PATH(resolved_path, path_length)) {
		/* skip UNC name */
		resolved_path[0] = DEFAULT_SLASH;
		resolved_path[1] = DEFAULT_SLASH;
		start = 2;
		while (!IS_SLASH(resolved_path[start])) {
			if (resolved_path[start] == 0) {
				goto verify;
			}
			resolved_path[start] = toupper(resolved_path[start]);
			start++;
		}
		resolved_path[start++] = DEFAULT_SLASH;
		while (!IS_SLASH(resolved_path[start])) {
			if (resolved_path[start] == 0) {
				goto verify;
			}
			resolved_path[start] = toupper(resolved_path[start]);
			start++;
		}
		resolved_path[start++] = DEFAULT_SLASH;
	} else if (IS_ABSOLUTE_PATH(resolved_path, path_length)) {
		/* skip DRIVE name */
		resolved_path[0] = toupper(resolved_path[0]);
		resolved_path[2] = DEFAULT_SLASH;
		start = 3;
	}
#elif defined(NETWARE)
	if (IS_ABSOLUTE_PATH(resolved_path, path_length)) {
		/* skip VOLUME name */
		start = 0;
		while (start != ':') {
			if (resolved_path[start] == 0) return -1;
			start++;
		}
		start++;
		if (!IS_SLASH(resolved_path[start])) return -1;
		resolved_path[start++] = DEFAULT_SLASH;
	}
#endif

	add_slash = (use_realpath != CWD_REALPATH) && path_length > 0 && IS_SLASH(resolved_path[path_length-1]);
	t = CWDG(realpath_cache_ttl) ? 0 : -1;
	path_length = tsrm_realpath_r(resolved_path, start, path_length, &ll, &t, use_realpath, 0, NULL TSRMLS_CC);

	if (path_length < 0) {
		errno = ENOENT;
		return 1;
	}

	if (!start && !path_length) {
		resolved_path[path_length++] = '.';
	}
	if (add_slash && path_length && !IS_SLASH(resolved_path[path_length-1])) {
		if (path_length >= MAXPATHLEN-1) {
			return -1;
		}
		resolved_path[path_length++] = DEFAULT_SLASH;
	}
	resolved_path[path_length] = 0;

#ifdef TSRM_WIN32
verify:
#endif
	if (verify_path) {
		cwd_state old_state;

		CWD_STATE_COPY(&old_state, state);
		state->cwd_length = path_length;

		tmp = realloc(state->cwd, state->cwd_length+1);
		if (tmp == NULL) {
#if VIRTUAL_CWD_DEBUG
			fprintf (stderr, "Out of memory\n");
#endif
			return 1;
		}
		state->cwd = (char *) tmp;

		memcpy(state->cwd, resolved_path, state->cwd_length+1);
		if (verify_path(state)) {
			CWD_STATE_FREE(state);
			*state = old_state;
			ret = 1;
		} else {
			CWD_STATE_FREE(&old_state);
			ret = 0;
		}
	} else {
		state->cwd_length = path_length;
		tmp = realloc(state->cwd, state->cwd_length+1);
		if (tmp == NULL) {
#if VIRTUAL_CWD_DEBUG
			fprintf (stderr, "Out of memory\n");
#endif
			return 1;
		}
		state->cwd = (char *) tmp;

		memcpy(state->cwd, resolved_path, state->cwd_length+1);
		ret = 0;
	}

#if VIRTUAL_CWD_DEBUG
	fprintf (stderr, "virtual_file_ex() = %s\n",state->cwd);
#endif
	return (ret);
}
/* }}} */
