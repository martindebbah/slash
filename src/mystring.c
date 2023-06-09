#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mystring.h"

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
  if(nchars < 0)
    return;

  if (nchars > str->length)
    nchars = str->length;
  str->data[str->length-nchars] = 0;
  str->length -= nchars;
}

struct string *string_truncate_token_and_spaces(struct string *str, size_t nchars) {
  if (nchars < 0)
    return str;

  if (nchars > str -> length)
    nchars = str -> length;

  struct string *newStr = string_new(str -> capacity);
  string_append(newStr, str -> data + nchars);
  string_delete(str);

  int i = 0;
  while (newStr -> data[i] == ' ')
    i++;
  if (i == 0)
    return newStr;
  struct string *finalStr = string_new(newStr -> capacity);
  string_append(finalStr, newStr -> data + i);
  string_delete(newStr);
  return finalStr;
}

void string_truncate_where(struct string * str, char delimit) {
  char *tmp = str->data;
  if(strchr(tmp, delimit) == NULL) return;
  while(tmp[0] != delimit){
      tmp++;
  }
  string_truncate(str, strlen(tmp));
}

char *copy(struct string *path) {
  if (path -> length == 0) {
    string_delete(path);
    return NULL;
  }
  int l = path -> length;
  char *ret = calloc(l + 1, 1);
  memcpy(ret, path -> data, l);
  string_delete(path);
  return ret;
}

struct string *clean(struct string *path) {
    char *tmp = path -> data;
    while(tmp[0] != '\0') {
      if(tmp[0] == '/' && tmp[1] == '/'){
        memmove(tmp, tmp+1, strlen(tmp));
      }
      else tmp++;
    }
    return path;
}
