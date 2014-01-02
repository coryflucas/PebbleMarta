char *strtok2(char *s, char delim) {
  char *p = s;
  while(*p != delim && *p != '\0') {
  	p++;
  }

  // if we hit end of string, return it, not whats next
  if(*p == '\0') {
  	return p;
  }

  *p++ = '\0';
  return p;
}