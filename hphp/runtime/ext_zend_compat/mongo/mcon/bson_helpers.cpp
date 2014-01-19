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
#include "types.h"
#include "bson_helpers.h"
#include "str.h"

void mcon_serialize_int(struct mcon_str *str, int num)
{
	int i = MONGO_32(num);

	mcon_str_addl(str, (char*) &i, 4, 0);
}

void mcon_serialize_int32(struct mcon_str *str, int num)
{
	int i = MONGO_32(num);

	mcon_str_addl(str, (char*) &i, 4, 0);
}

void mcon_serialize_int64(struct mcon_str *str, int64_t num)
{
	int64_t i = MONGO_64(num);

	mcon_str_addl(str, (char*) &i, 8, 0);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
