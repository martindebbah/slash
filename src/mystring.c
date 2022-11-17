#include <stdlib.h>
#include <string.h>

#include "mystring.h"

// TODO: implémenter les fonctions déclarées dans mystring.h

struct string * string_new(size_t capacity) {
  if (capacity == 0) return NULL;
  char* data = malloc(capacity);
  if (!data) { return NULL; }
  struct string * str = malloc(sizeof(*str));
  if (!str) {
    free(data);
    return NULL;
  }
  
  str->capacity = capacity;
  str->length = 0;
  str->data = data;
  str->data[0] = 0;
  return str;
}

void string_delete(struct string * str) {
  free(str->data);
  free(str);
}

int string_append (struct string * dest, char * src) {
  size_t src_len = strlen(src);
  if (dest->length + src_len + 1 > dest->capacity) return 0;
  memmove(dest->data+dest->length, src, src_len+1);
  dest->length += src_len;
  return 1;
}

int string_prepend(struct string * dest, char * src){
  size_t src_len = strlen(src);
  if(dest->length + src_len + 1 > dest->capacity) return 1;
  memmove(dest->data+src_len, dest->data, dest->length+1);
  memmove(dest->data, src, src_len);
  dest->length += src_len;
  return 0;
}


void string_truncate (struct string * str, size_t nchars) {
  if (nchars > str->length) { nchars = str->length; }
  str->data[str->length-nchars] = 0;
  str->length -= nchars;
}

char *copy(struct string *path) {
  int l = path -> length;
  char *ret = malloc(l + 1);
  memcpy(ret, path -> data, l);
  string_delete(path);
  if (!ret) {
    return NULL;
  }
  ret[l + 1] = '\0';
  return ret;
}
