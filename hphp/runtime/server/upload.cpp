/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/server/upload.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/util/logger.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/util/text-util.h"
#include "hphp/runtime/base/request-event-handler.h"

using std::set;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static void destroy_uploaded_files();

struct Rfc1867Data final : RequestEventHandler {
  std::set<std::string> rfc1867ProtectedVariables;
  std::set<std::string> rfc1867UploadedFiles;
  apc_rfc1867_data rfc1867ApcData;
  int (*rfc1867Callback)(apc_rfc1867_data *rfc1867ApcData,
                         unsigned int event, void *event_data, void **extra);
  void requestInit() override {
    if (RuntimeOption::EnableUploadProgress) {
      rfc1867Callback = apc_rfc1867_progress;
    } else {
      rfc1867Callback = nullptr;
    }
  }
  void requestShutdown() override {
    if (!rfc1867UploadedFiles.empty()) destroy_uploaded_files();
  }
};
IMPLEMENT_STATIC_REQUEST_LOCAL(Rfc1867Data, s_rfc1867_data);

/*
 *  This product includes software developed by the Apache Group
 *  for use in the Apache HTTP server project (http://www.apache.org/).
 *
 */

static void safe_php_register_variable(char *var, const Variant& val,
                                       Array& track_vars_array,
                                       bool override_protection);

#define FAILURE -1

/* The longest property name we use in an uploaded file array */
#define MAX_SIZE_OF_INDEX sizeof("[tmp_name]")

/* The longest anonymous name */
#define MAX_SIZE_ANONNAME 33

/* Errors */
#define UPLOAD_ERROR_OK   0  /* File upload succesful */
#define UPLOAD_ERROR_A    1  /* Uploaded file exceeded upload_max_filesize */
#define UPLOAD_ERROR_B    2  /* Uploaded file exceeded MAX_FILE_SIZE */
#define UPLOAD_ERROR_C    3  /* Partially uploaded */
#define UPLOAD_ERROR_D    4  /* No file uploaded */
#define UPLOAD_ERROR_E    6  /* Missing /tmp or similar directory */
#define UPLOAD_ERROR_F    7  /* Failed to write file to disk */
#define UPLOAD_ERROR_X    8  /* File upload stopped by extension */

static void normalize_protected_variable(char *varname) {
  char *s=varname, *index=nullptr, *indexend=nullptr, *p;

  /* overjump leading space */
  while (*s == ' ') {
    s++;
  }

  /* and remove it */
  if (s != varname) {
    memmove(varname, s, strlen(s)+1);
  }

  for (p=varname; *p && *p != '['; p++) {
    switch(*p) {
      case ' ':
      case '.':
        *p='_';
        break;
    }
  }

  /* find index */
  index = strchr(varname, '[');
  if (index) {
    index++;
    s=index;
  } else {
    return;
  }

  /* done? */
  while (index) {

    while (*index == ' ' || *index == '\r' ||
           *index == '\n' || *index=='\t') {
      index++;
    }
    indexend = strchr(index, ']');
    indexend = indexend ? indexend + 1 : index + strlen(index);

    if (s != index) {
      memmove(s, index, strlen(index)+1);
      s += indexend-index;
    } else {
      s = indexend;
    }

    if (*s == '[') {
      s++;
      index = s;
    } else {
      index = nullptr;
    }
  }
  *s++='\0';
}


static void add_protected_variable(char *varname) {
  normalize_protected_variable(varname);
  s_rfc1867_data->rfc1867ProtectedVariables.insert(varname);
}


static bool is_protected_variable(char *varname) {
  normalize_protected_variable(varname);
  auto iter = s_rfc1867_data->rfc1867ProtectedVariables.find(varname);
  return iter != s_rfc1867_data->rfc1867ProtectedVariables.end();
}


static void safe_php_register_variable(char *var, const Variant& val,
                                       Array& track_vars_array,
                                       bool override_protection) {
  if (override_protection || !is_protected_variable(var)) {
    register_variable(track_vars_array, var, val);
  }
}

bool is_uploaded_file(const std::string filename) {
  std::set<std::string> &rfc1867UploadedFiles =
    s_rfc1867_data->rfc1867UploadedFiles;
  return rfc1867UploadedFiles.find(filename) != rfc1867UploadedFiles.end();
}

const std::set<std::string> &get_uploaded_files() {
  return s_rfc1867_data->rfc1867UploadedFiles;
}

static void destroy_uploaded_files() {
  std::set<std::string> &rfc1867UploadedFiles =
    s_rfc1867_data->rfc1867UploadedFiles;
  for (auto iter = rfc1867UploadedFiles.begin();
       iter != rfc1867UploadedFiles.end(); iter++) {
    unlink(iter->c_str());
  }
  rfc1867UploadedFiles.clear();
}


/*
 *  Following code is based on apache_multipart_buffer.c from
 *  libapreq-0.33 package.
 *
 */

#define FILLUNIT (1024 * 5)

typedef struct {
  Transport *transport;

  /* read buffer */
  char *buffer;
  char *buf_begin;
  uint32_t  bufsize;
  int64_t   bytes_in_buffer; // signed to catch underflow errors

  /* boundary info */
  char *boundary;
  char *boundary_next;
  int  boundary_next_len;

  /* post data */
  const char *post_data;
  uint64_t post_size;
  uint64_t throw_size; // sum of all previously read chunks
  char *cursor;
  uint64_t read_post_bytes;
} multipart_buffer;

typedef std::list<std::pair<std::string, std::string> > header_list;

static uint32_t read_post(multipart_buffer *self, char *buf,
                          uint32_t bytes_to_read) {
  always_assert(bytes_to_read > 0);
  always_assert(self->post_data);
  always_assert(self->cursor >= self->post_data);
  int64_t bytes_remaining = (self->post_size - self->throw_size) -
                            (self->cursor - self->post_data);
  always_assert(bytes_remaining >= 0);
  if (bytes_to_read <= bytes_remaining) {
    memcpy(buf, self->cursor, bytes_to_read);
    self->cursor += bytes_to_read;
    return bytes_to_read;
  }

  uint32_t bytes_read = bytes_remaining;
  memcpy(buf, self->cursor, bytes_remaining);
  bytes_to_read -= bytes_remaining;
  always_assert(self->cursor = (char *)self->post_data +
                        (self->post_size - self->throw_size));
  while (bytes_to_read > 0 && self->transport->hasMorePostData()) {
    int extra_byte_read = 0;
    const void *extra = self->transport->getMorePostData(extra_byte_read);
    if (extra_byte_read == 0) break;
    if (RuntimeOption::AlwaysPopulateRawPostData) {
      // Possible overflow in buffer_append if post_size + extra_byte_read >=
      // MAX INT
      self->post_data = (const char *)buffer_append(
        self->post_data, self->post_size, extra, extra_byte_read);
      self->cursor = (char*)self->post_data + self->post_size;
    } else {
      self->post_data = (const char *)extra;
      self->throw_size = self->post_size;
      self->cursor = (char*)self->post_data;
    }
    self->post_size += extra_byte_read;
    if (bytes_to_read <= extra_byte_read) {
      memcpy(buf + bytes_read, self->cursor, bytes_to_read);
      self->cursor += bytes_to_read;
      return bytes_read + bytes_to_read;
    }
    memcpy(buf + bytes_read, self->cursor, extra_byte_read);
    bytes_to_read -= extra_byte_read;
    bytes_read += extra_byte_read;
  }
  return bytes_read;
}


/*
  fill up the buffer with client data.
  returns number of bytes added to buffer.
*/
static uint32_t fill_buffer(multipart_buffer *self) {
  uint32_t bytes_to_read, total_read = 0, actual_read = 0;

  /* shift the existing data if necessary */
  if (self->bytes_in_buffer > 0 && self->buf_begin != self->buffer) {
    memmove(self->buffer, self->buf_begin, self->bytes_in_buffer);
  }

  self->buf_begin = self->buffer;

  /* calculate the free space in the buffer */
  bytes_to_read = self->bufsize - self->bytes_in_buffer;
  always_assert(self->bufsize > 0);
  always_assert(self->bytes_in_buffer >= 0);
  /* read the required number of bytes */
  while (bytes_to_read > 0) {

    char *buf = self->buffer + self->bytes_in_buffer;

    actual_read = read_post(self, buf, bytes_to_read);

    /* update the buffer length */
    if (actual_read > 0) {
      always_assert(bytes_to_read >= actual_read);
      self->bytes_in_buffer += actual_read;
      self->read_post_bytes += actual_read;
      total_read += actual_read;
      bytes_to_read -= actual_read;
    } else {
      break;
    }
  }

  return total_read;
}


/* eof if we are out of bytes, or if we hit the final boundary */
static int multipart_buffer_eof(multipart_buffer *self) {
  if ( (self->bytes_in_buffer == 0 && fill_buffer(self) < 1) ) {
    return 1;
  } else {
    return 0;
  }
}


/* create new multipart_buffer structure */
static multipart_buffer *multipart_buffer_new(Transport *transport,
                                              const char *data, int size,
                                              std::string boundary) {
  multipart_buffer *self =
    (multipart_buffer *)calloc(1, sizeof(multipart_buffer));

  self->transport = transport;
  int minsize = boundary.length() + 6;
  if (minsize < FILLUNIT) minsize = FILLUNIT;

  self->buffer = (char *) calloc(1, minsize + 1);
  self->bufsize = minsize;

  vspprintf(&self->boundary, 0, "--%s", boundary.c_str());

  self->boundary_next_len =
    vspprintf(&self->boundary_next, 0, "\n--%s", boundary.c_str());

  self->buf_begin = self->buffer;
  self->bytes_in_buffer = 0;

  self->post_data = data;
  self->cursor = (char*)self->post_data;
  self->post_size = size;
  self->throw_size = 0;
  return self;
}


/*
  gets the next CRLF terminated line from the input buffer.
  if it doesn't find a CRLF, and the buffer isn't completely full, returns
  NULL; otherwise, returns the beginning of the null-terminated line,
  minus the CRLF.

  note that we really just look for LF terminated lines. this works
  around a bug in internet explorer for the macintosh which sends mime
  boundaries that are only LF terminated when you use an image submit
  button in a multipart/form-data form.
 */
static char *next_line(multipart_buffer *self) {
  /* look for LF in the data */
  char* line = self->buf_begin;
  char* ptr = (char*)memchr(self->buf_begin, '\n', self->bytes_in_buffer);

  if (ptr) {  /* LF found */

    /* terminate the string, remove CRLF */
    if ((ptr - line) > 0 && *(ptr-1) == '\r') {
      *(ptr-1) = 0;
    } else {
      *ptr = 0;
    }

    /* bump the pointer */
    self->buf_begin = ptr + 1;
    self->bytes_in_buffer -= (self->buf_begin - line);

  } else {  /* no LF found */

    /* buffer isn't completely full, fail */
    if (self->bytes_in_buffer < self->bufsize) {
      return nullptr;
    }
    /* return entire buffer as a partial line */
    line[self->bufsize] = 0;
    self->buf_begin = ptr;
    self->bytes_in_buffer = 0;
  }

  return line;
}


/* returns the next CRLF terminated line from the client */
static char *get_line(multipart_buffer *self) {
  char* ptr = next_line(self);

  if (!ptr) {
    fill_buffer(self);
    ptr = next_line(self);
  }

  return ptr;
}


/* finds a boundary */
static int find_boundary(multipart_buffer *self, char *boundary) {
  char *line;

  /* loop thru lines */
  while( (line = get_line(self)) )
  {
    /* finished if we found the boundary */
    if (!strcmp(line, boundary)) {
      return 1;
    }
  }

  /* didn't find the boundary */
  return 0;
}


/* parse headers */
static int multipart_buffer_headers(multipart_buffer *self,
                                    header_list &header) {
  char *line;
  std::pair<std::string, std::string> prev_entry;
  std::pair<std::string, std::string> entry;

  /* didn't find boundary, abort */
  if (!find_boundary(self, self->boundary)) {
    return 0;
  }

  /* get lines of text, or CRLF_CRLF */

  while( (line = get_line(self)) && strlen(line) > 0 )
  {
    /* add header to table */

    char *key = line;
    char *value = nullptr;

    /* space in the beginning means same header */
    if (!isspace(line[0])) {
      value = strchr(line, ':');
    }

    if (value) {
      *value = 0;
      do { value++; } while(isspace(*value));
      entry = std::make_pair(key, value);
    } else if (!header.empty()) {
      /* If no ':' on the line, add to previous line */
      entry = std::make_pair(prev_entry.first, prev_entry.second + line);
      header.pop_back();
    } else {
      continue;
    }

    header.push_back(entry);
    prev_entry = entry;
  }

  return 1;
}


static char *php_mime_get_hdr_value(header_list &header, char *key) {
  if (key == nullptr) return nullptr;
  for (header_list::iterator iter = header.begin();
       iter != header.end(); iter++) {
    if (!strcasecmp(iter->first.c_str(), key)) {
      return (char *)iter->second.c_str();
    }
  }
  return nullptr;
}


static char *php_ap_getword(char **line, char stop) {
  char *pos = *line, quote;
  char *res;

  while (*pos && *pos != stop) {

    if ((quote = *pos) == '"' || quote == '\'') {
      ++pos;
      while (*pos && *pos != quote) {
        if (*pos == '\\' && pos[1] && pos[1] == quote) {
          pos += 2;
        } else {
          ++pos;
        }
      }
      if (*pos) {
        ++pos;
      }
    } else ++pos;

  }
  if (*pos == '\0') {
    res = strdup(*line);
    *line += strlen(*line);
    return res;
  }

  res = (char*)malloc(pos - *line + 1);
  memcpy(res, *line, pos - *line);
  res[pos - *line] = 0;
  while (*pos == stop) {
    ++pos;
  }

  *line = pos;
  return res;
}


static char *substring_conf(char *start, int len, char quote) {
  char *result = (char *)malloc(len + 2);
  char *resp = result;
  int i;

  for (i = 0; i < len; ++i) {
    if (start[i] == '\\' &&
        (start[i + 1] == '\\' || (quote && start[i + 1] == quote))) {
      *resp++ = start[++i];
    } else {
      *resp++ = start[i];
    }
  }

  *resp++ = '\0';
  return result;
}


static char *php_ap_getword_conf(char **line) {
  char *str = *line, *strend, *res, quote;

  while (*str && isspace(*str)) {
    ++str;
  }

  if (!*str) {
    *line = str;
    return strdup("");
  }

  if ((quote = *str) == '"' || quote == '\'') {
    strend = str + 1;
look_for_quote:
    while (*strend && *strend != quote) {
      if (*strend == '\\' && strend[1] && strend[1] == quote) {
        strend += 2;
      } else {
        ++strend;
      }
    }
    if (*strend && *strend == quote) {
      char p = *(strend + 1);
      if (p != '\r' && p != '\n' && p != '\0') {
        strend++;
        goto look_for_quote;
      }
    }

    res = substring_conf(str + 1, strend - str - 1, quote);

    if (*strend == quote) {
      ++strend;
    }

  } else {

    strend = str;
    while (*strend && !isspace(*strend)) {
      ++strend;
    }
    res = substring_conf(str, strend - str, 0);
  }

  while (*strend && isspace(*strend)) {
    ++strend;
  }

  *line = strend;
  return res;
}


/*
  search for a string in a fixed-length byte string.
  if partial is true, partial matches are allowed at the end of the buffer.
  returns NULL if not found, or a pointer to the start of the first match.
*/
static char *php_ap_memstr(char *haystack, int haystacklen, char *needle,
                           int needlen, int partial) {
  int len = haystacklen;
  char *ptr = haystack;

  /* iterate through first character matches */
  while( (ptr = (char *)memchr(ptr, needle[0], len)) ) {

    /* calculate length after match */
    len = haystacklen - (ptr - (char *)haystack);

    /* done if matches up to capacity of buffer */
    if (memcmp(needle, ptr, needlen < len ? needlen : len) == 0 &&
        (partial || len >= needlen)) {
      break;
    }

    /* next character */
    ptr++; len--;
  }

  return ptr;
}


/* read until a boundary condition */
static int multipart_buffer_read(multipart_buffer *self, char *buf,
                                 int bytes, int *end) {
  int len, max;
  char *bound;

  /* fill buffer if needed */
  if (bytes > self->bytes_in_buffer) {
    fill_buffer(self);
  }

  /* look for a potential boundary match, only read data up to that point */
  if ((bound =
       php_ap_memstr(self->buf_begin, self->bytes_in_buffer,
                     self->boundary_next, self->boundary_next_len, 1))) {
    max = bound - self->buf_begin;
    if (end &&
        php_ap_memstr(self->buf_begin, self->bytes_in_buffer,
                      self->boundary_next, self->boundary_next_len, 0)) {
      *end = 1;
    }
  } else {
    max = self->bytes_in_buffer;
  }

  /* maximum number of bytes we are reading */
  len = max < bytes-1 ? max : bytes-1;

  /* if we read any data... */
  if (len > 0) {

    /* copy the data */
    memcpy(buf, self->buf_begin, len);
    buf[len] = 0;

    if (bound && len > 0 && buf[len-1] == '\r') {
      buf[--len] = 0;
    }

    /* update the buffer */
    self->bytes_in_buffer -= len;
    self->buf_begin += len;
  }

  return len;
}


/*
  XXX: this is horrible memory-usage-wise, but we only expect
  to do this on small pieces of form data.
*/
static char *multipart_buffer_read_body(multipart_buffer *self,
                                        unsigned int *len) {
  char buf[FILLUNIT], *out=nullptr;
  int total_bytes=0, read_bytes=0;

  while((read_bytes = multipart_buffer_read(self, buf, sizeof(buf), nullptr))) {
    out = (char *)realloc(out, total_bytes + read_bytes + 1);
    memcpy(out + total_bytes, buf, read_bytes);
    total_bytes += read_bytes;
  }

  if (out) out[total_bytes] = '\0';
  *len = total_bytes;

  return out;
}

/*
 * The combined READER/HANDLER
 *
 */

void rfc1867PostHandler(Transport* transport,
                        Array& post,
                        Array& files,
                        int content_length,
                        const void*& data, int& size,
                        const std::string boundary) {
  char *s=nullptr, *start_arr=nullptr;
  std::string array_index, abuf;
  char *temp_filename=nullptr, *lbuf=nullptr;
  int total_bytes=0, cancel_upload=0, is_arr_upload=0, array_len=0;
  int max_file_size=0, skip_upload=0, anonindex=0, is_anonymous;
  std::set<std::string> &uploaded_files = s_rfc1867_data->rfc1867UploadedFiles;
  multipart_buffer *mbuff;
  int fd=-1;
  void *event_extra_data = nullptr;
  unsigned int llen = 0;

  /* Initialize the buffer */
  if (!(mbuff = multipart_buffer_new(transport,
                                     (const char *)data, size, boundary))) {
    Logger::Warning("Unable to initialize the input buffer");
    return;
  }

  /* Initialize $_FILES[] */
  s_rfc1867_data->rfc1867ProtectedVariables.clear();

  uploaded_files.clear();

  int (*php_rfc1867_callback)(apc_rfc1867_data *rfc1867ApcData,
                              unsigned int event, void *event_data,
                              void **extra) = s_rfc1867_data->rfc1867Callback;

  if (php_rfc1867_callback != nullptr) {
    multipart_event_start event_start;

    event_start.content_length = content_length;
    if (php_rfc1867_callback(&s_rfc1867_data->rfc1867ApcData,
                             MULTIPART_EVENT_START, &event_start,
                             &event_extra_data) == FAILURE) {
      goto fileupload_done;
    }
  }

  while (!multipart_buffer_eof(mbuff)) {
    char buff[FILLUNIT];
    char *cd=nullptr,*param=nullptr,*filename=nullptr, *tmp=nullptr;
    size_t blen=0, wlen=0;
    off_t offset;

    header_list header;
    if (!multipart_buffer_headers(mbuff, header)) {
      goto fileupload_done;
    }

    if ((cd = php_mime_get_hdr_value(header, "Content-Disposition"))) {
      char *pair=nullptr;
      int end=0;

      while (isspace(*cd)) {
        ++cd;
      }

      while (*cd && (pair = php_ap_getword(&cd, ';')))
      {
        char *key=nullptr, *word = pair;

        while (isspace(*cd)) {
          ++cd;
        }

        if (strchr(pair, '=')) {
          key = php_ap_getword(&pair, '=');

          if (!strcasecmp(key, "name")) {
            if (param) {
              free(param);
            }
            param = php_ap_getword_conf(&pair);
          } else if (!strcasecmp(key, "filename")) {
            if (filename) {
              free(filename);
            }
            filename = php_ap_getword_conf(&pair);
          }
        }
        if (key) free(key);
        free(word);
      }

      /* Normal form variable, safe to read all data into memory */
      if (!filename && param) {
        unsigned int value_len;
        char *value = multipart_buffer_read_body(mbuff, &value_len);
        unsigned int new_val_len; /* Dummy variable */

        if (!value) {
          value = strdup("");
        }

        new_val_len = value_len;
        if (php_rfc1867_callback != nullptr) {
          multipart_event_formdata event_formdata;
          size_t newlength = 0;

          event_formdata.post_bytes_processed = mbuff->read_post_bytes;
          event_formdata.name = param;
          event_formdata.value = &value;
          event_formdata.length = new_val_len;
          event_formdata.newlength = &newlength;
          if (php_rfc1867_callback(&s_rfc1867_data->rfc1867ApcData,
                                   MULTIPART_EVENT_FORMDATA, &event_formdata,
                                   &event_extra_data) == FAILURE) {
            free(param);
            free(value);
            continue;
          }
          new_val_len = newlength;
        }

        String val(value, new_val_len, CopyString);
        safe_php_register_variable(param, val, post, 0);

        if (!strcasecmp(param, "MAX_FILE_SIZE")) {
          max_file_size = atol(value);
        }

        free(param);
        free(value);
        continue;
      }

      /* If file_uploads=off, skip the file part */
      if (!RuntimeOption::EnableFileUploads) {
        skip_upload = 1;
      }

      /* Return with an error if the posted data is garbled */
      if (!param && !filename) {
        Logger::Warning("File Upload Mime headers garbled");
        goto fileupload_done;
      }

      if (!param) {
        is_anonymous = 1;
        param = (char*)malloc(MAX_SIZE_ANONNAME);
        snprintf(param, MAX_SIZE_ANONNAME, "%u", anonindex++);
      } else {
        is_anonymous = 0;
      }

      /* New Rule: never repair potential malicious user input */
      if (!skip_upload) {
        char *tmp = param;
        long c = 0;

        while (*tmp) {
          if (*tmp == '[') {
            c++;
          } else if (*tmp == ']') {
            c--;
            if (tmp[1] && tmp[1] != '[') {
              skip_upload = 1;
              break;
            }
          }
          if (c < 0) {
            skip_upload = 1;
            break;
          }
          tmp++;
        }
      }

      total_bytes = cancel_upload = 0;

      if (!skip_upload) {
        /* Handle file */
        char path[PATH_MAX];

        // open a temporary file
        snprintf(path, sizeof(path), "%s/XXXXXX",
                 RuntimeOption::UploadTmpDir.c_str());
        fd = mkstemp(path);
        if (fd == -1) {
          Logger::Warning("Unable to open temporary file");
          Logger::Warning("File upload error - unable to create a "
                          "temporary file");
          cancel_upload = UPLOAD_ERROR_E;
        }
        temp_filename = strdup(path);
      }

      if (!skip_upload && php_rfc1867_callback != nullptr) {
        multipart_event_file_start event_file_start;

        event_file_start.post_bytes_processed = mbuff->read_post_bytes;
        event_file_start.name = param;
        event_file_start.filename = &filename;
        if (php_rfc1867_callback(&s_rfc1867_data->rfc1867ApcData,
                                 MULTIPART_EVENT_FILE_START,
                                 &event_file_start,
                                 &event_extra_data) == FAILURE) {
          if (temp_filename) {
            if (cancel_upload != UPLOAD_ERROR_E) { /* file creation failed */
              close(fd);
              unlink(temp_filename);
            }
            free(temp_filename);
          }
          temp_filename="";
          free(param);
          free(filename);
          continue;
        }
      }


      if (skip_upload) {
        free(param);
        free(filename);
        continue;
      }

      if (strlen(filename) == 0) {
        Logger::Verbose("No file uploaded");
        cancel_upload = UPLOAD_ERROR_D;
      }

      offset = 0;
      end = 0;
      while (!cancel_upload &&
             (blen = multipart_buffer_read(mbuff, buff, sizeof(buff), &end)))
      {
        if (php_rfc1867_callback != nullptr) {
          multipart_event_file_data event_file_data;

          event_file_data.post_bytes_processed = mbuff->read_post_bytes;
          event_file_data.offset = offset;
          event_file_data.data = buff;
          event_file_data.length = blen;
          event_file_data.newlength = &blen;
          if (php_rfc1867_callback(&s_rfc1867_data->rfc1867ApcData,
                                   MULTIPART_EVENT_FILE_DATA,
                                   &event_file_data,
                                   &event_extra_data) == FAILURE) {
            cancel_upload = UPLOAD_ERROR_X;
            continue;
          }
        }


        if (VirtualHost::GetUploadMaxFileSize() > 0 &&
            total_bytes > VirtualHost::GetUploadMaxFileSize()) {
          Logger::Verbose("upload_max_filesize of %" PRId64 " bytes exceeded "
                          "- file [%s=%s] not saved",
                          VirtualHost::GetUploadMaxFileSize(),
                          param, filename);
          cancel_upload = UPLOAD_ERROR_A;
        } else if (max_file_size && (total_bytes > max_file_size)) {
          Logger::Verbose("MAX_FILE_SIZE of %d bytes exceeded - "
                          "file [%s=%s] not saved",
                          max_file_size, param, filename);
          cancel_upload = UPLOAD_ERROR_B;
        } else if (blen > 0) {

          wlen = write(fd, buff, blen);

          if (wlen < blen) {
            Logger::Verbose("Only %zd bytes were written, expected to "
                            "write %zd", wlen, blen);
            cancel_upload = UPLOAD_ERROR_F;
          } else {
            total_bytes += wlen;
          }

          offset += wlen;
        }
      }
      if (fd!=-1) { /* may not be initialized if file could not be created */
        close(fd);
      }
      if (!cancel_upload && !end) {
        Logger::Verbose("Missing mime boundary at the end of the data for "
                        "file %s", strlen(filename) > 0 ? filename : "");
        cancel_upload = UPLOAD_ERROR_C;
      }
      if (strlen(filename) > 0 && total_bytes == 0 && !cancel_upload) {
        Logger::Verbose("Uploaded file size 0 - file [%s=%s] not saved",
                        param, filename);
        cancel_upload = 5;
      }

      if (php_rfc1867_callback != nullptr) {
        multipart_event_file_end event_file_end;

        event_file_end.post_bytes_processed = mbuff->read_post_bytes;
        event_file_end.temp_filename = temp_filename;
        event_file_end.cancel_upload = cancel_upload;
        if (php_rfc1867_callback(&s_rfc1867_data->rfc1867ApcData,
                                 MULTIPART_EVENT_FILE_END,
                                 &event_file_end,
                                 &event_extra_data) == FAILURE) {
          cancel_upload = UPLOAD_ERROR_X;
        }
      }

      if (cancel_upload && cancel_upload != UPLOAD_ERROR_C) {
        if (temp_filename) {
          if (cancel_upload != UPLOAD_ERROR_E) { /* file creation failed */
            unlink(temp_filename);
          }
          free(temp_filename);
        }
        temp_filename="";
      } else {
        s_rfc1867_data->rfc1867UploadedFiles.insert(temp_filename);
      }

      /* is_arr_upload is true when name of file upload field
       * ends in [.*]
       * start_arr is set to point to 1st [
       */
      is_arr_upload = (start_arr = strchr(param,'[')) &&
                      (param[strlen(param)-1] == ']');

      if (is_arr_upload) {
        array_len = strlen(start_arr);
        array_index = std::string(start_arr+1, array_len-2);
      }

      /* Add $foo_name */
      if (llen < strlen(param) + MAX_SIZE_OF_INDEX + 1) {
        llen = strlen(param);
        lbuf = (char *) realloc(lbuf, llen + MAX_SIZE_OF_INDEX + 1);
        llen += MAX_SIZE_OF_INDEX + 1;
      }

      if (is_arr_upload) {
        abuf = std::string(param, strlen(param)-array_len);
        snprintf(lbuf, llen, "%s_name[%s]",
                 abuf.c_str(), array_index.c_str());
      } else {
        snprintf(lbuf, llen, "%s_name", param);
      }

      /* The \ check should technically be needed for win32 systems only
       * where it is a valid path separator. However, IE in all it's wisdom
       * always sends the full path of the file on the user's filesystem,
       * which means that unless the user does basename() they get a bogus
       * file name. Until IE's user base drops to nill or problem is fixed
       * this code must remain enabled for all systems.
       */
      s = strrchr(filename, '\\');
      if ((tmp = strrchr(filename, '/')) > s) {
        s = tmp;
      }

      Array globals(get_global_variables());
      if (!is_anonymous) {
        if (s && s > filename) {
          String val(s+1, strlen(s+1), CopyString);
          safe_php_register_variable(lbuf, val, globals, 0);
        } else {
          String val(filename, strlen(filename), CopyString);
          safe_php_register_variable(lbuf, val, globals, 0);
        }
      }

      /* Add $foo[name] */
      if (is_arr_upload) {
        snprintf(lbuf, llen, "%s[name][%s]",
                 abuf.c_str(), array_index.c_str());
      } else {
        snprintf(lbuf, llen, "%s[name]", param);
      }
      if (s && s > filename) {
        String val(s+1, strlen(s+1), CopyString);
        safe_php_register_variable(lbuf, val, files, 0);
      } else {
        String val(filename, strlen(filename), CopyString);
        safe_php_register_variable(lbuf, val, files, 0);
      }
      free(filename);
      s = nullptr;

      /* Possible Content-Type: */
      if ((cancel_upload && cancel_upload != UPLOAD_ERROR_C) ||
          !(cd = php_mime_get_hdr_value(header, "Content-Type"))) {
        cd = "";
      } else {
        /* fix for Opera 6.01 */
        s = strchr(cd, ';');
        if (s != nullptr) {
          *s = '\0';
        }
      }

      /* Add $foo_type */
      if (is_arr_upload) {
        snprintf(lbuf, llen, "%s_type[%s]",
                 abuf.c_str(), array_index.c_str());
      } else {
        snprintf(lbuf, llen, "%s_type", param);
      }
      if (!is_anonymous) {
        String val(cd, strlen(cd), CopyString);
        safe_php_register_variable(lbuf, val, globals, 0);
      }

      /* Add $foo[type] */
      if (is_arr_upload) {
        snprintf(lbuf, llen, "%s[type][%s]",
                 abuf.c_str(), array_index.c_str());
      } else {
        snprintf(lbuf, llen, "%s[type]", param);
      }
      String val(cd, strlen(cd), CopyString);
      safe_php_register_variable(lbuf, val, files, 0);

      /* Restore Content-Type Header */
      if (s != nullptr) {
        *s = ';';
      }
      s = "";

      /* Initialize variables */
      add_protected_variable(param);

      /* if param is of form xxx[.*] this will cut it to xxx */
      if (!is_anonymous) {
        String val(temp_filename, strlen(temp_filename), CopyString);
        safe_php_register_variable(param, val, globals, 1);
      }

      /* Add $foo[tmp_name] */
      if (is_arr_upload) {
        snprintf(lbuf, llen, "%s[tmp_name][%s]",
                 abuf.c_str(), array_index.c_str());
      } else {
        snprintf(lbuf, llen, "%s[tmp_name]", param);
      }
      add_protected_variable(lbuf);
      String tempFileName(temp_filename, strlen(temp_filename), CopyString);
      safe_php_register_variable(lbuf, tempFileName, files, 1);

      Variant file_size, error_type;

      error_type = cancel_upload;

      /* Add $foo[error] */
      if (cancel_upload) {
        file_size = 0;
      } else {
        file_size = total_bytes;
      }

      if (is_arr_upload) {
        snprintf(lbuf, llen, "%s[error][%s]",
                 abuf.c_str(), array_index.c_str());
      } else {
        snprintf(lbuf, llen, "%s[error]", param);
      }
      safe_php_register_variable(lbuf, error_type, files, 0);

      /* Add $foo_size */
      if (is_arr_upload) {
        snprintf(lbuf, llen, "%s_size[%s]",
                 abuf.c_str(), array_index.c_str());
      } else {
        snprintf(lbuf, llen, "%s_size", param);
      }
      if (!is_anonymous) {
        safe_php_register_variable(lbuf, file_size, globals, 0);
      }

      /* Add $foo[size] */
      if (is_arr_upload) {
        snprintf(lbuf, llen, "%s[size][%s]",
                 abuf.c_str(), array_index.c_str());
      } else {
        snprintf(lbuf, llen, "%s[size]", param);
      }
      safe_php_register_variable(lbuf, file_size, files, 0);
      free(param);
    }
  }
fileupload_done:
  data = mbuff->post_data;
  size = mbuff->post_size;
  if (php_rfc1867_callback != nullptr) {
    multipart_event_end event_end;

    event_end.post_bytes_processed = mbuff->read_post_bytes;
    php_rfc1867_callback(&s_rfc1867_data->rfc1867ApcData,
                         MULTIPART_EVENT_END, &event_end, &event_extra_data);
  }
  if (lbuf) free(lbuf);
  s_rfc1867_data->rfc1867ProtectedVariables.clear();
  if (mbuff->boundary_next) free(mbuff->boundary_next);
  if (mbuff->boundary) free(mbuff->boundary);
  if (mbuff->buffer) free(mbuff->buffer);
  if (mbuff) free(mbuff);
}

///////////////////////////////////////////////////////////////////////////////
}
