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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "types.h"

void mcon_str_add(mcon_str *xs, char *str, int f)
{
	mcon_str_addl(xs, str, strlen(str), f);
}

void mcon_str_addl(mcon_str *xs, char *str, int le, int f)
{
	if (xs->l + le > xs->a - 1) {
		xs->d = (char*) realloc(xs->d, xs->a + le + MCON_STR_PREALLOC);
		xs->a = xs->a + le + MCON_STR_PREALLOC;
	}
	if (!xs->l) {
		xs->d[0] = '\0';
	}
	memcpy(xs->d + xs->l, str, le);
	xs->d[xs->l + le] = '\0';
	xs->l = xs->l + le;

	if (f) {
		free(str);
	}
}

void mcon_str_add_int(mcon_str *xs, int i)
{
	char *tmp = (char*) calloc(1, 11);

	snprintf(tmp, 10, "%d", i);
	mcon_str_add(xs, tmp, 1);
}

void mcon_str_free(mcon_str *s)
{
	if (s->d) {
		free(s->d);
	}
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
