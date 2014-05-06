#include "hashtable.h"
#include "stdio.h"
#include "stdlib.h"
#include "game.h" //TA BORT!!
int Hashtable::count(){
  return tot_elements;
}

Hashtable::Hashtable(int itable_size){
  if (itable_size > 0){
    table_size = itable_size;
  }
  else{
    fprintf(stderr,"Error Hashtable::Hashtable() nonpositive value as "
            "ihash_table\n");
    exit(1);
  }
  h_table = new Linkedlist*[table_size];
  if (h_table == NULL){
    fprintf(stderr,"Failed to allocate memory!\n");
    exit(1);
  }
  for (int i = 0; i < table_size; i++)
    h_table[i] = NULL;

  lowest_index = itable_size - 1; /* yes, the lowest is the max, and vice versa
                                     this is because nothing is in the h_table
                                  */
  highest_index = 0;
  tot_elements = 0;
}

Hashtable::~Hashtable(){
  Linkedlist *last, *curr;
  for (int i = 0; i < table_size; i++){
    if (h_table[i] != NULL){
      curr = h_table[i]->next;
      last = h_table[i];
      while(curr != NULL){
        delete last;
        last = curr;
        curr = curr->next;
      }
      delete last;
    }
  }
  delete [] h_table;
}

int Hashtable::hash(int value){
  return value % table_size;
}

int Hashtable::insert( int new_val ){
  int h = hash(new_val);
  Linkedlist *last = NULL;
  Linkedlist *put = h_table[h];
  while (put){
    if (put->value == new_val){
      return 1;
    }
    last = put;
    put = put->next;
  }
  put = new Linkedlist;
  if (put == NULL){
    fprintf(stderr,"Failed to allocate memory!\n");
    exit(1);
  }
  if (last != NULL)
    last->next = put;
  else
    h_table[h] = put;

  put->value = new_val;
  put->next = NULL;
  tot_elements ++;
  if (h < lowest_index)
    lowest_index = h;
  if (h > highest_index)
    highest_index = h;

  return 0;
}

int Hashtable::takeout(int &ret){
  int h;
  if (tot_elements == 0)
    return 1;

  h = lowest_index;
  Linkedlist *get = h_table[h];

  ret = get->value;
  h_table[h] = get->next;
  tot_elements --;
  if (tot_elements > 0){
    while(h_table[lowest_index] == NULL){
      lowest_index ++;
    }
  }
  else{
    lowest_index = table_size - 1;
    highest_index = 0;
  }
  delete get;
  return 0;
}
