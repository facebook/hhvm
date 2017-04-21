/*-
 * Copyright (c) 2008 Christos Zoulas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: readcdf.c,v 1.33 2012/06/20 21:52:36 christos Exp $")
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "cdf.h"
#include "magic.h"

#include "hphp/util/bstring.h"

#include <folly/portability/Unistd.h>

#define NOTMIME(ms) (((ms)->flags & MAGIC_MIME) == 0)

static const struct nv {
  const char *pattern;
  const char *mime;
} name2mime[] = {
  { "WordDocument",   "msword",   },
  { "PowerPoint",     "vnd.ms-powerpoint",  },
  { "DigitalSignature",   "vnd.ms-msi",   },
  { nullptr,       nullptr,     },
}, name2desc[] = {
  { "WordDocument",   "Microsoft Office Word",},
  { "PowerPoint",     "Microsoft PowerPoint", },
  { "DigitalSignature",   "Microsoft Installer",  },
  { nullptr,   nullptr},
};

static const struct cv {
  uint64_t clsid[2];
  const char *mime;
} clsid2mime[] = {
  {
    { 0x00000000000c1084LLU, 0x46000000000000c0LLU },
    "x-msi",
  }
}, clsid2desc[] = {
  {
    { 0x00000000000c1084LLU, 0x46000000000000c0LLU },
    "MSI Installer",
  }
};

private const char *
cdf_clsid_to_mime(const uint64_t clsid[2], const struct cv *cv) {
  if (clsid[0] == cv->clsid[0] && clsid[1] == cv->clsid[1])
    return cv->mime;
  return nullptr;
}

private const char *
cdf_app_to_mime(const char *vbuf, const struct nv *nv)
{
  size_t i;
  const char *rv = nullptr;
#ifdef USE_C_LOCALE
  locale_t old_lc_ctype, c_lc_ctype;

  c_lc_ctype = newlocale(LC_CTYPE_MASK, "C", 0);
  assert(c_lc_ctype != nullptr);
  old_lc_ctype = uselocale(c_lc_ctype);
  assert(old_lc_ctype != nullptr);
#endif
  for (i = 0; nv[i].pattern != nullptr; i++)
    if (HPHP::bstrcasestr(
        vbuf,
        strlen(vbuf),
        nv[i].pattern,
        strlen(nv[i].pattern)) != nullptr) {
      rv = nv[i].mime;
      break;
    }
#ifdef CDF_DEBUG
  fprintf(stderr, "unknown app %s\n", vbuf);
#endif
#ifdef USE_C_LOCALE
  (void)uselocale(old_lc_ctype);
  freelocale(c_lc_ctype);
#endif
  return rv;
}

private int
cdf_file_property_info(struct magic_set *ms, const cdf_property_info_t *info,
    size_t count, const uint64_t clsid[2])
{
        size_t i;
        cdf_timestamp_t tp;
        struct timeval ts;
        char buf[64];
        const char *str = nullptr;
        const char *s;
        int len;

        if (!NOTMIME(ms)) {
          str = cdf_clsid_to_mime(clsid, clsid2mime);
        }

        for (i = 0; i < count; i++) {
                cdf_print_property_name(buf, sizeof(buf), info[i].pi_id);
                switch (info[i].pi_type) {
                case CDF_NULL:
                        break;
                case CDF_SIGNED16:
                        if (NOTMIME(ms) && file_printf(ms, ", %s: %hd", buf,
                            info[i].pi_s16) == -1)
                                return -1;
                        break;
                case CDF_SIGNED32:
                        if (NOTMIME(ms) && file_printf(ms, ", %s: %d", buf,
                            info[i].pi_s32) == -1)
                                return -1;
                        break;
                case CDF_UNSIGNED32:
                        if (NOTMIME(ms) && file_printf(ms, ", %s: %u", buf,
                            info[i].pi_u32) == -1)
                                return -1;
                        break;
                case CDF_FLOAT:
                        if (NOTMIME(ms) && file_printf(ms, ", %s: %g", buf,
                            info[i].pi_f) == -1)
                                return -1;
                        break;
                case CDF_DOUBLE:
                        if (NOTMIME(ms) && file_printf(ms, ", %s: %g", buf,
                            info[i].pi_d) == -1)
                                return -1;
                        break;
                case CDF_LENGTH32_STRING:
                case CDF_LENGTH32_WSTRING:
                        len = info[i].pi_str.s_len;
                        if (len > 1) {
                                char vbuf[1024];
                                size_t j, k = 1;

                                if (info[i].pi_type == CDF_LENGTH32_WSTRING)
                                    k++;
                                s = info[i].pi_str.s_buf;
                                for (j = 0; j < sizeof(vbuf) && len--;
                                    j++, s += k) {
                                        if (*s == '\0')
                                                break;
                                        if (isprint((unsigned char)*s))
                                                vbuf[j] = *s;
                                }
                                if (j == sizeof(vbuf))
                                        --j;
                                vbuf[j] = '\0';
                                if (NOTMIME(ms)) {
                                        if (vbuf[0]) {
                                                if (file_printf(ms, ", %s: %s",
                                                    buf, vbuf) == -1)
                                                        return -1;
                                        }
                                } else if (str == nullptr && info[i].pi_id ==
                                        CDF_PROPERTY_NAME_OF_APPLICATION) {
                                        if (strstr(vbuf, "Word"))
                                                str = "msword";
                                        else if (strstr(vbuf, "Excel"))
                                                str = "vnd.ms-excel";
                                        else if (strstr(vbuf, "Powerpoint"))
                                                str = "vnd.ms-powerpoint";
                                        else if (strstr(vbuf,
                                            "Crystal Reports"))
                                                str = "x-rpt";
                                }
                        }
                        break;
                case CDF_FILETIME:
                        tp = info[i].pi_tp;
                        if (tp != 0) {
              char tbuf[64];
#if defined(PHP_WIN32) && _MSC_VER <= 1500
              if (tp < 1000000000000000i64) {
#else
              if (tp < 1000000000000000LL) {
#endif
                                        cdf_print_elapsed_time(tbuf,
                                            sizeof(tbuf), tp);
                                        if (NOTMIME(ms) && file_printf(ms,
                                            ", %s: %s", buf, tbuf) == -1)
                                                return -1;
                                } else {
                                        char *c, *ec;
                                        if (cdf_timestamp_to_timespec(&ts, tp) == -1) {
                      return -1;
                    }
                                        time_t tmp = ts.tv_sec;
                                        c = cdf_ctime(&tmp, tbuf);
                                        if ((ec = strchr(c, '\n')) != nullptr)
                                                *ec = '\0';

                                        if (NOTMIME(ms) && file_printf(ms,
                                            ", %s: %s", buf, c) == -1)
                                                return -1;
                                }
                        }
                        break;
                case CDF_CLIPBOARD:
                        break;
                default:
                        return -1;
                }
        }
        if (!NOTMIME(ms)) {
    if (str == nullptr)
      return 0;
                if (file_printf(ms, "application/%s", str) == -1)
                        return -1;
        }
        return 1;
}

private int
cdf_file_summary_info(struct magic_set *ms, const cdf_header_t *h,
    const cdf_stream_t *sst, const uint64_t clsid[2])
{
        cdf_summary_info_header_t si;
        cdf_property_info_t *info;
        size_t count;
        int m;

        if (cdf_unpack_summary_info(sst, h, &si, &info, &count) == -1)
                return -1;

        if (NOTMIME(ms)) {
                const char *str;
                if (file_printf(ms, "Composite Document File V2 Document")
        == -1)
                        return -1;

                if (file_printf(ms, ", %s Endian",
                    si.si_byte_order == 0xfffe ?  "Little" : "Big") == -1)
                        return -2;
                switch (si.si_os) {
                case 2:
                        if (file_printf(ms, ", Os: Windows, Version %d.%d",
                            si.si_os_version & 0xff,
                            (uint32_t)si.si_os_version >> 8) == -1)
                                return -2;
                        break;
                case 1:
                        if (file_printf(ms, ", Os: MacOS, Version %d.%d",
                            (uint32_t)si.si_os_version >> 8,
                            si.si_os_version & 0xff) == -1)
                                return -2;
                        break;
                default:
                        if (file_printf(ms, ", Os %d, Version: %d.%d", si.si_os,
                            si.si_os_version & 0xff,
                            (uint32_t)si.si_os_version >> 8) == -1)
                                return -2;
                        break;
                }
                str = cdf_clsid_to_mime(clsid, clsid2desc);
                if (str) {
                  if (file_printf(ms, ",%s", str) == -1) {
                    return -2;
                  }
                }
        }

        m = cdf_file_property_info(ms, info, count, clsid);
        free(info);

        return m == -1 ? -2 : m;
}

#ifdef notdef
private char *
format_clsid(char *buf, size_t len, const uint64_t uuid[2]) {
    snprintf(buf, len, "%.8" PRIx64 "-%.4" PRIx64 "-%.4" PRIx64 "-%.4"
              PRIx64 "-%.12" PRIx64,
              (uuid[0] >> 32) & (uint64_t)0x000000000ffffffffLLU,
              (uuid[0] >> 16) & (uint64_t)0x0000000000000ffffLLU,
              (uuid[0] >>  0) & (uint64_t)0x0000000000000ffffLLU,
              (uuid[1] >> 48) & (uint64_t)0x0000000000000ffffLLU,
              (uuid[1] >>  0) & (uint64_t)0x0000fffffffffffffLLU);
    return buf;
}
#endif


protected int
file_trycdf(struct magic_set *ms, int fd, const unsigned char *buf,
    size_t nbytes)
{
        cdf_info_t info;
        cdf_header_t h;
        cdf_sat_t sat, ssat;
        cdf_stream_t sst, scn;
        cdf_dir_t dir;
        int i;
        const char *expn = "";
        const char *corrupt = "corrupt: ";

        info.i_fd = fd;
        info.i_buf = buf;
        info.i_len = nbytes;
        if (ms->flags & MAGIC_APPLE)
                return 0;
        if (cdf_read_header(&info, &h) == -1)
                return 0;
#ifdef CDF_DEBUG
        cdf_dump_header(&h);
#endif

        if ((i = cdf_read_sat(&info, &h, &sat)) == -1) {
                expn = "Can't read SAT";
                goto out0;
        }
#ifdef CDF_DEBUG
        cdf_dump_sat("SAT", &sat, CDF_SEC_SIZE(&h));
#endif

        if ((i = cdf_read_ssat(&info, &h, &sat, &ssat)) == -1) {
                expn = "Can't read SSAT";
                goto out1;
        }
#ifdef CDF_DEBUG
        cdf_dump_sat("SSAT", &ssat, CDF_SHORT_SEC_SIZE(&h));
#endif

        if ((i = cdf_read_dir(&info, &h, &sat, &dir)) == -1) {
                expn = "Can't read directory";
                goto out2;
        }

        const cdf_directory_t *root_storage;
        if ((i = cdf_read_short_stream(&info, &h, &sat, &dir, &sst,
            &root_storage)) == -1) {
                expn = "Cannot read short stream";
                goto out3;
        }
#ifdef CDF_DEBUG
        cdf_dump_dir(&info, &h, &sat, &ssat, &sst, &dir);
#endif
#ifdef notdef
        if (root_storage) {
          if (NOTMIME(ms)) {
            char clsbuf[128];
            if (file_printf(ms, "CLSID %s, ",
                  format_clsid(clsbuf, sizeof(clsbuf),
                    root_storage->d_storage_uuid)) == -1)
              return -1;
          }
        }
#endif
        if ((i = cdf_read_summary_info(&info, &h, &sat, &ssat, &sst, &dir,
            &scn)) == -1) {
                if (errno == ESRCH) {
                        corrupt = expn;
                        expn = "No summary info";
                } else {
                        expn = "Cannot read summary info";
                }
                goto out4;
        }
#ifdef CDF_DEBUG
        cdf_dump_summary_info(&h, &scn);
#endif
       if ((i = cdf_file_summary_info(ms, &h, &scn,
               root_storage->d_storage_uuid)) < 0)
                expn = "Can't expand summary_info";

       if (i == 0) {
         const char *str = nullptr;
         cdf_directory_t *d;
         char name[__arraycount(d->d_name)];
         size_t j, k;

         for (j = 0; str == nullptr && j < dir.dir_len; j++) {
           d = &dir.dir_tab[j];
           for (k = 0; k < sizeof(name); k++)
             name[k] = (char)cdf_tole2(d->d_name[k]);
           str = cdf_app_to_mime(name,
               NOTMIME(ms) ? name2desc : name2mime);
         }
         if (NOTMIME(ms)) {
           if (str != nullptr) {
             if (file_printf(ms, "%s", str) == -1)
               return -1;
             i = 1;
           }
         } else {
           if (str == nullptr)
             str = "vnd.ms-office";
           if (file_printf(ms, "application/%s", str) == -1)
             return -1;
           i = 1;
         }
       }
       free(scn.sst_tab);
out4:
        free(sst.sst_tab);
out3:
        free(dir.dir_tab);
out2:
        free(ssat.sat_tab);
out1:
        free(sat.sat_tab);
out0:
        if (i != 1) {
    if (i == -1) {
        if (NOTMIME(ms)) {
      if (file_printf(ms,
          "Composite Document File V2 Document") == -1)
          return -1;
                if (*expn)
                        if (file_printf(ms, ", %s%s", corrupt, expn) == -1)
                                return -1;
        } else {
      if (file_printf(ms, "application/CDFV2-corrupt") == -1)
          return -1;
        }
    }
                i = 1;
        }
        return i;
}
