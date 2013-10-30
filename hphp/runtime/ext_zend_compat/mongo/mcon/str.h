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
#ifndef __HAVE_MCON_STR_H__
#define __HAVE_MCON_STR_H__

#define MCON_STR_PREALLOC 1024
#define mcon_str_ptr_init(str) str = (mcon_str*) malloc(sizeof(mcon_str)); str->l = 0; str->a = 0; str->d = NULL;
#define mcon_str_ptr_dtor(str) free(str->d); free(str)
#define mcon_str_dtor(str)     free(str.d)

typedef struct mcon_str {
	int   l;
	int   a;
	char *d;
} mcon_str;

void mcon_str_add(mcon_str *xs, char *str, int f);
void mcon_str_addl(mcon_str *xs, char *str, int le, int f);
void mcon_str_add_int(mcon_str *xs, int i);
void mcon_str_free(mcon_str *s);

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
