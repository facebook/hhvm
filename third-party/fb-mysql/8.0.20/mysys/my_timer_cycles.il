/* Copyright (c) 2008 MySQL AB, 2009 Sun Microsystems, Inc.
   Use is subject to license terms.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/* Sun Studio SPARC inline templates for cycle timer */
/* Sun Studio i386 and x86_64 inline templates for cycle timer */
/* I didn't say ".volatile" or ".nonvolatile". */

.inline my_timer_cycles_il_sparc64,0
rd %tick,%o0
.end

.inline my_timer_cycles_il_sparc32,0
rd %tick,%o2
srlx %o2,32,%o0
sra %o2,0,%o1
.end

.inline my_timer_cycles_il_i386,0
rdtsc
.end

.inline my_timer_cycles_il_x86_64,0
rdtsc
shlq $32,%rdx
orq %rdx,%rax
.end
