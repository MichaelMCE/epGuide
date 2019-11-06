
//  Copyright (c) Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.




#include "../common.h"



static inline TTREE *treeAlloc ()
{
	return my_calloc(1, sizeof(TTREE));
}

static inline TTREEENTRY *treeEntryAlloc ()
{
	return my_calloc(1, sizeof(TTREEENTRY));
}

static inline void treeInsertTail (TTREEENTRY *entry, TLISTITEM *item)
{
	// add last
	if (entry->tail)
		listInsert(item, entry->tail, NULL);
	else
		entry->head = item;
	entry->tail = item;
	if (!entry->head) entry->head = item;
}

static inline void treeInsertBefore (TTREEENTRY *entry, TLISTITEM *item, TLISTITEM *newItem)
{
	listInsert(newItem, item->prev, item);
	if (entry->head == item) entry->head = newItem;
	else if (!entry->head) entry->head = newItem;
}

static inline void treeInsertAfter (TTREEENTRY *entry, TLISTITEM *item, TLISTITEM *newItem)
{
	listInsert(newItem, item, item->next);
	if (entry->tail == item) entry->tail = newItem;
}

static inline TTREEENTRY *treeEntryAddItem (TTREEENTRY *entry, const char *name, const int64_t id, const int64_t type)
{
	TLISTITEM *item = listNewItem((void*)name);
	
	TTREEENTRY *subentry = treeEntryAlloc();
	subentry->type = type;
	subentry->name = my_strdup(name);
	subentry->id = id;
	subentry->parentId = entry->id;
	subentry->head = NULL;
	listSetStorage(item, subentry);
	
	treeInsertTail(entry, item);
	return subentry;
}

int treeCountItems (TTREE *tree, const int64_t id)
{
	TTREEENTRY *entry = treeEntryFind(tree->root, id);
	return treeEntryGetSubItemCount(entry);
}

TTREEENTRY *treeEntryFindEx (TTREEENTRY *entry, const int64_t id, TTREEENTRY **parent, TLISTITEM **subitem)
{
	if (entry->id == id) return entry;
	
	if (treeEntryIsBranch(entry)){
		TLISTITEM *item = entry->head;
		while(item){
			TTREEENTRY *found = treeEntryFindEx(listGetStorage(item), id, parent, subitem);
			if (found){
				if (parent && !*parent) *parent = entry;
				if (subitem && !*subitem) *subitem = item;
				return found;
			}
			
			item = item->next;
		}
	}
	return NULL;
}

TTREEENTRY *treeEntryFind (TTREEENTRY *entry, const int64_t id)
{
	if (entry->id == id) return entry;
	
	if (treeEntryIsBranch(entry)){
		TLISTITEM *item = entry->head;
		while(item){
			TTREEENTRY *found = treeEntryFind(listGetStorage(item), id);
			if (found) return found;
			
			item = item->next;
		}
	}
	return NULL;
}

TTREEENTRY *treeListGetSubEntry (TLISTITEM *subitem)
{
	return listGetStorage(subitem);
}

int treeEntryGetSubItemCount (TTREEENTRY *entry)
{
	return listCount(entry->head);
}

static inline int64_t treeEntryGetId (TTREEENTRY *entry)
{
	return entry->id;
}

void treeEntrySetStorage (TTREEENTRY *entry, void *data)
{
	entry->storage = data;
}

void *treeEntryGetStorage (TTREEENTRY *entry)
{
	return entry->storage;
}

void treeSetStorage (TTREE *tree, const int64_t id, void *data)
{
	TTREEENTRY *entry = treeEntryFind(tree->root, id);
	if (entry) treeEntrySetStorage(entry, data);
}

void *treeGetStorage (TTREE *tree, const int64_t id)
{
	TTREEENTRY *entry = treeEntryFind(tree->root, id);
	if (entry) return treeEntryGetStorage(entry);
	
	return NULL;
}

static inline TTREEENTRY *treeNodeFind (TTREEENTRY *entry, const int64_t nodeId, const int depth)
{
	if (depth < 1) return NULL;
	
	if (treeEntryIsBranch(entry)){
		if (entry->id == nodeId) return entry;
		
		TLISTITEM *item = entry->head;
		while(item){
			TTREEENTRY *found = treeNodeFind(listGetStorage(item), nodeId, depth-1);
			if (found) return found;
			
			item = item->next;
		}
	}
	return NULL;
}

TTREEENTRY *treeFind (TTREE *tree, const int64_t nodeId, const int64_t entryId, const int depth)
{
	TTREEENTRY *entry = treeNodeFind(tree->root, nodeId, depth);
	if (entry)
		return treeEntryFind(entry, entryId);
	return NULL;
}

TTREEENTRY *treeAddItem (TTREE *tree, const int64_t nodeId, const char *name, const int64_t id, const int64_t type)
{
	TTREEENTRY *entry = treeEntryFind(tree->root, nodeId);
	if (entry) return treeEntryAddItem(entry, name, id, type);
	return NULL;
}

static inline TTREEENTRY *treeAddItemBefore (TTREE *tree, const int64_t parentId, const int64_t subitemId, char *name, const int64_t id, const int64_t type)
{
	TTREEENTRY *entry = treeEntryFind(tree->root, parentId);
	if (!entry) return NULL;
	
	TLISTITEM *item = entry->head;
		
	while(item){
		TTREEENTRY *subitem = listGetStorage(item);
		
		if (subitem->id == subitemId){
			TLISTITEM *newItem = listNewItem((void*)name);
	
			TTREEENTRY *subentry = treeEntryAlloc();
			subentry->type = type;
			subentry->name = my_strdup(name);
			subentry->id = id;
			subentry->parentId = entry->id;
			subentry->head = NULL;
			listSetStorage(newItem, subentry);
				
			treeInsertBefore(entry, item, newItem);
			return subentry;
		}
		item = item->next;
	}
	return NULL;
}

static inline TTREEENTRY *treeAddItemAfter (TTREE *tree, const int64_t parentId, const int64_t subitemId, char *name, const int64_t id, const int64_t type)
{
	TTREEENTRY *entry = treeEntryFind(tree->root, parentId);
	if (!entry) return NULL;
	
	TLISTITEM *item = entry->head;
	while(item){
		TTREEENTRY *subitem = listGetStorage(item);
			
		if (subitem->id == subitemId){
			TLISTITEM *newItem = listNewItem((void*)name);
	
			TTREEENTRY *subentry = treeEntryAlloc();
			subentry->type = type;
			subentry->name = my_strdup(name);
			subentry->id = id;
			subentry->parentId = entry->id;
			subentry->head = NULL;
			listSetStorage(newItem, subentry);
				
			treeInsertAfter(entry, item, newItem);
			return subentry;
		}
		item = item->next;
	}
	return NULL;
}
static inline TTREEENTRY *treeAddEntryBefore (TTREE *tree, const int64_t subitemId, TTREEENTRY *subentry)
{
	TTREEENTRY *parent = NULL;
	TLISTITEM *item = NULL;

	TTREEENTRY *subitem = treeEntryFindEx(tree->root, subitemId, &parent, &item);
	if (!subitem || !parent) return NULL;
	
	TLISTITEM *newItem = listNewItem(subentry->name);
	subentry->parentId = parent->id;
	listSetStorage(newItem, subentry);
				
	treeInsertBefore(parent, item, newItem);
	return subentry;
}

static inline TTREEENTRY *treeAddEntryBeforeEx (TTREE *tree, const int64_t nodeId, const int64_t subitemId, TTREEENTRY *subentry, const int depth)
{
	TTREEENTRY *parent = NULL;
	TLISTITEM *item = NULL;
	
	TTREEENTRY *node = treeNodeFind(tree->root, nodeId, depth);
	if (!node) return NULL;
	
	TTREEENTRY *subitem = treeEntryFindEx(node, subitemId, &parent, &item);
	if (!subitem || !parent) return NULL;
	
	TLISTITEM *newItem = listNewItem(subentry->name);
	subentry->parentId = parent->id;
	listSetStorage(newItem, subentry);
				
	treeInsertBefore(parent, item, newItem);
	return subentry;
}

static inline TTREEENTRY *treeAddEntryAfter (TTREE *tree, const int64_t subitemId, TTREEENTRY *subentry)
{
	TTREEENTRY *parent = NULL;
	TLISTITEM *item = NULL;
	TTREEENTRY *subitem = treeEntryFindEx(tree->root, subitemId, &parent, &item);
	if (!subitem || !parent) return NULL;
	
	TLISTITEM *newItem = listNewItem(subentry->name);
	subentry->parentId = parent->id;
	listSetStorage(newItem, subentry);
				
	treeInsertAfter(parent, item, newItem);
	return subentry;
}

static inline void treeEntryDestroy (TTREEENTRY *entry)
{
	if (treeEntryIsBranch(entry)){
		TLISTITEM *item = entry->head;
		while(item){
			treeEntryDestroy(listGetStorage(item));
			item = item->next;
		}

		listDestroyAll(entry->head);
	}

	my_free(entry->name);
	my_free(entry->storage);
	my_free(entry);
}

void treeEntryDestroyItems (TTREEENTRY *entry)
{
	if (!entry) return;
	
	TLISTITEM *item = entry->head;
	if (item){
		while(item){
			treeEntryDestroy(listGetStorage(item));
			item = item->next;
		}
		listDestroyAll(entry->head);
	}
	entry->head = NULL;
	entry->tail = NULL;
}

static inline TTREEENTRY *treeEntryRemove (TTREE *tree, const int64_t id)
{
	TLISTITEM *item = NULL;
	TTREEENTRY *parent = NULL;
	TTREEENTRY *entry = treeEntryFindEx(tree->root, id, &parent, &item);
	if (!entry) return NULL;

	if (parent){
		if (parent->head == item)
			parent->head = item->next;
		if (parent->tail == item)
			parent->tail = item->prev;
	}

	if (item){
		listRemove(item);
		listDestroy(item);
	}
	return entry;
}

static inline TTREEENTRY *treeEntryRemoveEx (TTREE *tree, const int64_t nodeId, const int64_t id, const int depth)
{
	TLISTITEM *item = NULL;
	TTREEENTRY *parent = NULL;
	
	
	TTREEENTRY *node = treeNodeFind(tree->root, nodeId, depth);
	if (!node) return NULL;

	TTREEENTRY *entry = treeEntryFindEx(node, id, &parent, &item);
	if (!entry) return NULL;

	if (parent){
		if (parent->head == item)
			parent->head = item->next;
		if (parent->tail == item)
			parent->tail = item->prev;
	}

	if (item){
		listRemove(item);
		listDestroy(item);
	}
	return entry;
}

void treeEntryMoveEx (TTREE *tree, const int64_t nodeId, const int64_t fromId, const int64_t toId, const int depth)
{
	TTREEENTRY *entry = treeEntryRemoveEx(tree, nodeId, fromId, depth);
	if (entry)
		treeAddEntryBeforeEx(tree, nodeId, toId, entry, depth);
}

void treeDestoryItem (TTREE *tree, const int64_t id)
{
	//printf("@@@ treeDestoryItem %i\n", id);

	TTREEENTRY *entry = treeEntryRemove(tree, id);
	if (entry){
		treeEntryDestroy(entry);
		if (tree->root == entry){
			tree->root = NULL;
		}
	}
}

void treeEntryMove (TTREE *tree, const int64_t fromId, const int64_t toId)
{
	TTREEENTRY *entry = treeEntryRemove(tree, fromId);
	if (entry)
		treeAddEntryBefore(tree, toId, entry);
}

void treeEntryMoveLast (TTREE *tree, const int64_t fromId, const int64_t toId)
{
	TTREEENTRY *entry = treeEntryFind(tree->root, toId);
	
	TTREEENTRY *subEntry = treeEntryRemove(tree, fromId);
	subEntry->parentId = entry->id;
	
	TLISTITEM *item = listNewItem((void*)subEntry->name);
	listSetStorage(item, subEntry);
	treeInsertTail(entry, item);
}

TTREE *treeCreate (const char *name, const int64_t id)
{
	TTREE *tree = treeAlloc();
	
	tree->root = treeEntryAlloc();
	tree->root->type = TREE_TYPE_BRANCH;
	tree->root->name = my_strdup(name);
	tree->root->id = id;
	tree->root->parentId = 0;
	tree->root->head = NULL;
	
	tree->lock = lockCreate("treeCreate");
	return tree;	
}

void treeFree (TTREE *tree)
{
	lockWait(tree->lock, INFINITE);
	if (tree->root)
		treeEntryDestroy(tree->root);
	lockClose(tree->lock);
	my_free(tree);
}
