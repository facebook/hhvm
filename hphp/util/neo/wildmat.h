/*
 * Copyright (C) 1986-1991 Rich Salz <rsalz@osf.org>
 *
 */

#ifndef __WILDMAT_H_
#define __WILDMAT_H_ 1

__BEGIN_DECLS

int wildmat(const char *text, const char *p);
int wildmatcase(const char *text, const char *p);

__END_DECLS

#endif /* __WILDMAT_H_ */
