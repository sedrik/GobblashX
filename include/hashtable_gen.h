#ifndef __HASHTABLE_GEN_H__
#define __HASHTABLE_GEN_H__

#include "stdio.h"
#include "stdlib.h"

template <class TYP>
class Linkedlist_gen{
 public:
  TYP satellite_data;
  int value;
  Linkedlist_gen *next;
};

template <class TYP>
class Hashtable_gen{
 private:
  int table_size;
  int tot_elements;
  Linkedlist_gen<TYP> **h_table;
  int hash(int value);
 public:
  int insert(int new_val, const TYP &isatellite_data); /*does not allow clones.
                            returns 0 if everything was ok.
                            returns 1 if the value was already in h_table .*/
  int get(int val, TYP &isatellite_data); /* returns the satellite_data for the
                                             element with the given value
                                             returns 0 if everything was ok.
                                             1 if val is invalid */
  int count(); /* returns the total amount of elements */
  Hashtable_gen(int itable_size);
  ~Hashtable_gen();
};

/** This is ugly, but hell, templates demand having their declarations in the
    same file. or a file beeing included from it **/

template <class TYP>
int Hashtable_gen<TYP>::count(){
  return tot_elements;
}

template <class TYP>
Hashtable_gen<TYP>::Hashtable_gen(int itable_size){
  if (itable_size > 0){
    table_size = itable_size;
  }
  else{
    printf("Error Hashtable_gen::Hashtable_gen() nonpositive value as ihash_table\n");
    exit(1);
  }
  h_table = new Linkedlist_gen<TYP>*[table_size];
  for (int i = 0; i < table_size; i++)
    h_table[i] = NULL;

  tot_elements = 0;
}

template <class TYP>
Hashtable_gen<TYP>::~Hashtable_gen(){
  Linkedlist_gen<TYP> *last, *curr;
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

template <class TYP>
int Hashtable_gen<TYP>::hash(int value){
  return value % table_size;
}

template <class TYP>
int Hashtable_gen<TYP>::insert( int new_val, const TYP &isatellite_data ){
  int h = hash(new_val);
  Linkedlist_gen<TYP> *put = h_table[h];
  while (put){
    if (put->value == new_val)
      return 1;
    put = put->next;
  }
  put = new Linkedlist_gen<TYP>;
  put->next = h_table[h];
  put->value = new_val;
  put->satellite_data = isatellite_data;

  h_table[h] = put;
  tot_elements ++;
  return 0;
}

template <class TYP>
int Hashtable_gen<TYP>::get(int val, TYP &isatellite_data){
  int h;
  if (tot_elements == 0)
    return 1;

  h = hash(val);
  Linkedlist_gen<TYP> *get = h_table[h];

  while (get != NULL && get->value != val){
    get = get->next;
  }

  if (get == NULL)
    return 1;
  else{
    isatellite_data = get->satellite_data;
    return 0;
  }


#endif //__HASHTABLE_H__
}
