#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

class Linkedlist{
 public:
  int value;
  Linkedlist *next;
};

class Hashtable{
 private:
  int table_size;
  int tot_elements;
  Linkedlist **h_table;
  int hash(int value);

  int lowest_index; /* the id of the linked list in the hash_table that has
                         the lowest id in the hash_table. it's itable_size
                    -1 if there's nothing in the h_table */
  int highest_index;

 public:
  int insert(int new_val); /* does not allow clones.
                            returns 0 if everything was ok.
                            returns 1 if the value was already in h_table .*/
  int takeout(int &ret); /* takes out one element and removes it.
                 returns 0 if everything was ok. 1 if there was
                 nothing left */
  int count(); /* returns the total amount of elements */
  Hashtable(int itable_size);
  ~Hashtable();
};

#endif //__HASHTABLE_H__
