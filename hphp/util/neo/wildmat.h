/*
 * Copyright (C) 1986-1991 Rich Salz <rsalz@osf.org>
 *
 */

#ifndef incl_HPHP_WILDMAT_H_
#define incl_HPHP_WILDMAT_H_ 1

__BEGIN_DECLS

int wildmat(const char *text, const char *p);
int wildmatcase(const char *text, const char *p);

__END_DECLS

#endif /* incl_HPHP_WILDMAT_H_ */
