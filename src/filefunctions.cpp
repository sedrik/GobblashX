#include "filefunctions.h"
#include <fstream>

bool is_whitespace(char c){
  return c == ' ' || c == '\n' || c == '\t';
}

string first_word(string &s){
  int first = 0, n = 1;
  string ret;
  while( is_whitespace( s[first] ) && s[first] != '\0' )
    first ++;
  if ( s[first] == '\0' ){
    s = "";
    return "";
  }

  while( not is_whitespace( s[first + n] ) &&
         s[first + n] != '\0' ){
    n++;
  }

  ret = s.substr(first,n);

  int len = s.size() - n - 1 - first;
  if (len > 0)
    s = s.substr(n + 1 + first, len);
  else
    s = "";

  return ret;
}

/* safe_strlol, puts the number in s into store
   returns 1 if there was an errorous string and complains
   return 0 if everything went ok
 */
int safe_strtol(string s, int &store){
  char *errptr = NULL; //points to the invalid character
  store = strtol(s.c_str(), &errptr, 10);
  if (*errptr){
    printf("Errorous number - unexpected %c\n", *errptr);
    return 1;
  }
  return 0;
}

int safe_strtol(string s, bool &store){
  char *errptr = NULL; //points to the invalid character
  int temp;
  temp = strtol(s.c_str(), &errptr, 10);
  store = temp;
  if (*errptr){
    printf("Errorous number - unexpected %c\n", *errptr);
    return 1;
  }
  return 0;
}

int safe_strtol(string s, Sint16 &store){
  int val, retval;
  retval = safe_strtol(s, val);
  store = val;
  return retval;
}

int safe_strtol(string s, Uint16 &store){
  int val, retval;
  retval = safe_strtol(s, val);
  store = val;
  return retval;
}

string skipcomments(ifstream &file, string &row, int &rownum){
  string word;
  do{
    rownum ++;
    if (not getline(file, row)){
      return "<eof>";
    }
    word = first_word(row);
  }while(word[0] == '#' || word == "");
  return word;
}
