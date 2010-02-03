/*
 * Copyright 2001-2004 Brandon Long
 * All Rights Reserved.
 *
 * ClearSilver Templating System
 *
 * This code is made available under the terms of the ClearSilver License.
 * http://www.clearsilver.net/license.hdf
 *
 */

#ifndef __NEO_RAND_H_
#define __NEO_RAND_H_ 1

__BEGIN_DECLS

void neo_seed_rand (long int seed);
int neo_rand (int max);
int neo_rand_string (char *s, int slen);
int neo_rand_word (char *s, int slen);

__END_DECLS

#endif /* __NEO_RAND_H_ */
