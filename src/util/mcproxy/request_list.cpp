/*
 * Request list management  module.
 */
#include "proxy.h"
#include "request_int.h"
#include "gfuncs.h"

/* List of REQUEST_ENTRY; faster than releasing and allocating, even with GC */
static REQUEST_LIST_IMPL *freelist;


/*
 * Returns a new request list.
 */
REQUEST_LIST *
request_list_new()
{
  return (REQUEST_LIST *) gcalloc(sizeof(REQUEST_LIST_IMPL));
}

/*
 * Frees a request list.
 */
void
request_list_free(REQUEST_LIST *vlist)
{
  REQUEST_LIST_IMPL *list = (REQUEST_LIST_IMPL *)vlist;

  if (list->head != NULL)
  {
    HPHP::Logger::Verbose("Warning: freeing request list entries implicitly!");
    while (list->head != NULL)
    {
      REQUEST *req = request_list_next(vlist);
      request_free(req);
    }
  }

  gfree(vlist);
}


/*
 * Adds a REQEUST_ENTRY to a list.
 */
static void
request_list_add_entry(REQUEST_LIST_IMPL *list, REQUEST_ENTRY *entry)
{
  if (list->tail != NULL)
    list->tail->next = entry;
  else
    list->head = entry;
  list->tail = entry;
  entry->next = NULL;

  list->count++;
}


/*
 * Pops the next REQUEST_ENTRY off the head of a list. Does not add it
 * to the freelist or free it!
 */
static REQUEST_ENTRY *
request_list_next_entry(REQUEST_LIST_IMPL *list)
{
  REQUEST_ENTRY *entry;

  if (list->head == NULL)
    return NULL;
  entry = list->head;

  list->head = entry->next;
  if (list->head == NULL)
    list->tail = NULL;
  list->count--;

  return entry;
}


/*
 * Pops the next entry off the head of a list.
 */
REQUEST *
request_list_next(REQUEST_LIST *vlist)
{
  REQUEST_LIST_IMPL *list = (REQUEST_LIST_IMPL *)vlist;
  REQUEST_ENTRY *entry;
  REQUEST_IMPL *request;

  if (list->head == NULL)
    return NULL;
  entry = request_list_next_entry(list);
  request = entry->request;

  entry->request = NULL;
  request_list_add_entry(freelist, entry);

  return (REQUEST *) request;
}


/*
 * Returns the next entry off the head of a list, but doesn't remove it
 * from the list.
 */
REQUEST *
request_list_peek(REQUEST_LIST *vlist)
{
  REQUEST_LIST_IMPL *list = (REQUEST_LIST_IMPL *)vlist;

  if (list->head == NULL)
    return NULL;
  return (REQUEST *) list->head->request;
}


/*
 * Returns the entry at the tail of a list, but doesn't remove it
 * from the list.
 */
REQUEST *
request_list_peek_tail(REQUEST_LIST *vlist)
{
  REQUEST_LIST_IMPL *list = (REQUEST_LIST_IMPL *)vlist;

  if (list->tail == NULL)
    return NULL;
  return (REQUEST *) list->tail->request;
}


/*
 * Adds an entry to a list.
 */
void
request_list_add(REQUEST_LIST *vlist, REQUEST *vreq)
{
  REQUEST_LIST_IMPL *list = (REQUEST_LIST_IMPL *)vlist;
  REQUEST_IMPL *request = (REQUEST_IMPL *) vreq;
  REQUEST_ENTRY *entry;

  if (vlist == NULL)
  {
    HPHP::Logger::Error("Null request list");
    throw HPHP::Exception("abort");
  }

  entry = request_list_next_entry(freelist);
  if (entry == NULL)
    entry = (REQUEST_ENTRY *)gmalloc(sizeof(REQUEST_ENTRY));
  entry->request = request;
  entry->next = NULL;

  request_list_add_entry(list, entry);
}


/*
 * Removes all the entries from a list that have a given opaque argument.
 */
void
request_list_delete(REQUEST_LIST *vlist, void *arg)
{
  REQUEST_LIST_IMPL *list = (REQUEST_LIST_IMPL *)vlist;
  REQUEST_ENTRY **nextptr;

  for (nextptr = &list->head; *nextptr != NULL;)
  {
    if ((*nextptr)->request->arg == arg)
    {
      REQUEST_ENTRY *entry = *nextptr;
      *nextptr = entry->next;
      request_free((REQUEST *) entry->request);
      entry->request = NULL;
      request_list_add_entry(freelist, entry);
    }
    else
    {
      nextptr = &(*nextptr)->next;
    }

    if (*nextptr != NULL)
      list->tail = *nextptr;
  }

  if (list->head == NULL)
    list->tail = NULL;
}

/*
 * Initializes the request list module.
 */
void
request_list_init()
{
  freelist = (REQUEST_LIST_IMPL *) request_list_new();
}
