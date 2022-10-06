#include "string.h"


/** Copy string
+++ This function copies a source string into a destination buffer. 
+++ strncpy copies as many characters from src to dst as there is space
+++ in src. The string is padded with zeros. This way, buffer overflow 
+++ won't happen. If dst is longer than src, dst will be truncated. The 
+++ truncation will be detected and the program will interrupt.
--- dst:    destination buffer
--- size:   size of destination buffer
--- src:    source string
+++ Return: void
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++**/
void copy_string(char *dst, size_t size, const char *src){

  strncpy(dst, src, size);
  if (dst[size-1] != '\0'){
    printf("cannot copy, string too long:\n%s\n", src);
    exit(1);
  }

  return;
}

