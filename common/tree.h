
// libmylcd - http://mylcd.sourceforge.net/
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



#ifndef _TREE_H_
#define _TREE_H_


#define TREE_TYPE_BRANCH		1
#define TREE_TYPE_LEAF			2

#define treeEntryIsBranch(e)	(((TTREEENTRY*)(e))->type == TREE_TYPE_BRANCH)
#define treeEntryIsLeaf(e)		(((TTREEENTRY*)(e))->type == TREE_TYPE_LEAF)

typedef struct {
	int type;			// item or node branch
	int64_t id;

	char *name;
	void *storage;
	
	int parentId;
	TLISTITEM *head;
	TLISTITEM *tail;
}TTREEENTRY;



typedef struct {
	TTREEENTRY *root;
	TMLOCK *lock;
}TTREE;




TTREE *treeCreate (const char *name, const int64_t id);
void treeFree (TTREE *tree);

void treeEntrySetStorage (TTREEENTRY *entry, void *data);
void *treeEntryGetStorage (TTREEENTRY *entry);

TTREEENTRY *treeListGetSubEntry (TLISTITEM *subitem);
int treeEntryGetSubItemCount (TTREEENTRY *entry);
int treeCountItems (TTREE *tree, const int64_t id);

// create and add an item at this node/end of branch (nodeId)
// note that id's are created and maintained by _you_
TTREEENTRY *treeAddItem (TTREE *tree, const int64_t nodeId, const char *name, const int64_t id, const int64_t tree_type);

// first search for the nodeId, if found then within this the subId
TTREEENTRY *treeFind (TTREE *tree, const int64_t nodeId, const int64_t entryId, const int depth);

void treeDestoryItem (TTREE *tree, const int64_t id);
void treeEntryDestroyItems (TTREEENTRY *entry);

void treeEntryMove (TTREE *tree, const int64_t fromId, const int64_t toId);
void treeEntryMoveEx (TTREE *tree, const int64_t nodeId, const int64_t fromId, const int64_t toId, const int depth);
void treeEntryMoveLast (TTREE *tree, const int64_t fromId, const int64_t toId);	// insert in to branch toId

TTREEENTRY *treeEntryFind (TTREEENTRY *entry, const int64_t id);
TTREEENTRY *treeEntryFindEx (TTREEENTRY *entry, const int64_t id, TTREEENTRY **parent, TLISTITEM **subitem);


void *treeGetStorage (TTREE *tree, const int64_t id);
void treeSetStorage (TTREE *tree, const int64_t id, void *data);






#endif

