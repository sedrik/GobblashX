#ifndef __FILEFUNCTIONS_H__
#define __FILEFUNCTIONS_H__

#include <string>
#include <stdlib.h>
#include "SDL/SDL.h"

using namespace std;

/* is_whitespace, returns true iff c is considered a whitespace
*/
bool is_whitespace(char c);

/* first_word, returns the first word in s and removes it from s
*/
string first_word(string &s);

/* safe_strlol, puts the number in s into store
   returns 1 if there was an errorous string and complains
   return 0 if everything went ok
 */
int safe_strtol(string s, int &store);
int safe_strtol(string s, bool &store);
int safe_strtol(string s, Sint16 &store);
int safe_strtol(string s, Uint16 &store);

/* skipcomments, returns the first word of the first row in file.
   puts the rest of the first row in file into row, and skips all comments
   rownum will increase by 1
   if the file is empty it will return "<eof>"
*/
string skipcomments(ifstream &file, string &row, int &rownum);

#endif // __FILEFUNCTIONS_H__
