/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// pllist.h : Packed Linked List
// This data structure is used to create and manage nested lists of items
// of various size and containing any type of data.
// The list and its items are stored contiguosly in a unique block of memory.
// Items can only be appended to the end of the list.
// The memory allocation is increased in steps (whose size is set on list
// creation) as needed on item insertion.
// Item insertion and iteration through the items of the list is done
// via utility function that use callback procedures to let the user
// manage the various types of data stored in the list items.
// Items can only be accessed sequentially.
// --2003  - Alessandro Felice Cantatore
//===========================================================================


#ifndef _PLLIST_H_
   #define _PLLIST_H_

// packed linked list type definition ---------------------------------------

// packed linked list item handle
typedef ULONG HNDPLLITEM;// packed linked list node handle
typedef HNDPLLITEM * PHNDPLLITEM;

// packed linked list data type
typedef struct _PLLIST PLLIST;
typedef PLLIST* PPLLIST;

// packed linked list header ------------------------------------------------
struct _PLLIST {
   UINT cbTot    : 31;   // total list size
   UINT isNode   :  1;   // always TRUE as the list header is also a node
   ULONG offFirst;       // offset to the first item (list header size)
   ULONG offLast;
   ULONG offParent;      // offset (from the base address) of the parent node
   ULONG count;          // count of child items
   ULONG cbGrow;         // allocation grow step size (must be a power of 2)
};

// linked list item ---------------------------------------------------------
typedef struct {
   UINT offNext  : 31;   // offset (from the base address) to the next item
   UINT isNode   :  1;   // TRUE if the item is a node
} PLLITEM, * PPLLITEM;

// packed linked list node item ---------------------------------------------
typedef struct {
   UINT offNext  : 31;   // offset (from the base address) to the next item
   UINT isNode   :  1;   // TRUE if the item is a node
   ULONG offFirst;       // offset to the first item (list header size)
                         // (may be > sizeof(PLLNODE))
   ULONG offLast;        // offset to the last added item
   ULONG offParent;      // offset (from the base address) of the parent node
   ULONG count;          // count of child items
} PLLNODE, * PPLLNODE;

// iteration callback procedure data structure ------------------------------
// the address of this data is passed to the callback procedure as parameter
#pragma pack(1)
typedef struct {
   HNDPLLITEM hItem;     // current item handle
   PVOID pData;          // current item data
   PVOID pParm;          // user data (last parameter of PPLListIterate() )
   USHORT nestLevel;     // nesting level (0 for direct child of the list)
   UCHAR flag;           // recursion flag (see notes below)
   UCHAR context;        // tells in which context the procedure is called
                         // (see notes below)
} PLLITERDATA, * PPLLITERDATA;
#pragma pack()


// definitions

// Iteration options :
#define PPLLITER_NORECUR     0x0000       // only child of the current node
#define PPLLITER_RECUR       0x0001       // recursively process all items
// the following flags implicate recursion and can be ORed
#define PPLLITER_INCLNODE    0x0002       // include current node
#define PPLLITER_NODEPRE     0x0004       // execute node before child items
#define PPLLITER_NODEPOST    0x0008       // execute node after child items
/* Note:
 -1) If neither PPLLITER_NODEPRE nor PPLLITER_NODEPOST have been specified
     PPLLITER_NODEPRE is assumed (i.e. nodes are processed before theirs
     descendants)
 -2)
     the 'context' member of PLLITERDATA gets the following values:
     PPLLITER_NORECUR  : the current item is not a node or PPLListIterate()
                         'flag' parameter was PPLLITER_NORECUR
     PPLLITER_INCLNODE : the current data is the starting node data
     PPLLITER_NODEPRE  : the callback procedure is beeing called on a node,
                         the node children have not yet been processed
     PPLLITER_NODEPOST : the callback procedure is beeing called on a node,
                         all the node descendants have already been processed
*/


// iteration via PLListItemFirst() and PLListItemNext() ---------------------

typedef struct {
   union {
      HNDPLLITEM hNode;    // PLListItemFirst() node handle
      PPLLITEM pNext;      // next item address
   } ;
   PVOID pData;            // item data
} ITERITEM, * PITERITEM;

// standard callback procedures ---------------------------------------------

/*
 Add an item to the list.
 Parm 1 : the address of the storage reserved for the item data (PVOID).
 Parm 2 : user data (PVOID).
 When the first parameter is NULL the procedure must return the size of
 the storage needed by item data.
 The value returned when the first parameter is not NULL is not used.
*/
typedef ULONG (*PPLLISTADDITEM)(PVOID, PVOID);

/*
 Iteration callback procedure.
 Parm 1 : PPLLITERDATA (the item address, parent node address, nesting level,
                        user data 1, user data 2)
*/
typedef BOOL (*PPLLISTITERATE)(PPLLITERDATA);

// prototypes ---------------------------------------------------------------

PPLLIST PLListNew(ULONG cbGrow);
PPLLIST PLListDup(PPLLIST pList, ULONG cb);
PPLLIST PLListItemAdd(PPLLIST pList, HNDPLLITEM hNode, PHNDPLLITEM phItem,
                      PPLLISTADDITEM pFunc, PVOID pParm);
PPLLIST PLListNodeAdd(PPLLIST pList, HNDPLLITEM hNode, PHNDPLLITEM pNode,
                      PPLLISTADDITEM pFunc, PVOID pParm);
VOID PLListDel(PPLLIST plist);
PPLLIST PLListShrink(PPLLIST pList);
PPLLIST PLListListImport(PPLLIST pList, PPLLIST pImport, HNDPLLITEM hNode);
PPLLIST PLListExport(PPLLIST pList, HNDPLLITEM hNode,
                     BOOL inclNode, ULONG cbGrow);
ULONG PLListIterate(PPLLIST pList, HNDPLLITEM hNode, ULONG flag,
                    PPLLISTITERATE pFunc, PVOID pParm, ULONG ulMaxIterations);
ULONG PLListItemsCount(PPLLIST pList, HNDPLLITEM hNode, BOOL bRecur);
PVOID PLListItemData(PPLLIST pList, HNDPLLITEM hItem);
BOOL PLListItemFirst(PPLLIST pList, PITERITEM p);
BOOL PLListItemNext(PPLLIST pList, PITERITEM p);

// MACROS ------------------------------------------------------------------

// The following macros avoid casting

// ADD ITEM PROCEDURE (casts parameters to the correct type)
#define PLListItemAddC(_plist_, _hnode_, _phitem_, _pFunc_, _pparm_) \
   (PLListItemAdd((_plist_), (HNDPLLITEM)(_hnode_), (PHNDPLLITEM)(_phitem_), \
                    (PPLLISTADDITEM)(_pFunc_), (PVOID)(_pparm_)))
// ADD NODE PROCEDURE (casts parameters to the correct type)
#define PLListNodeAddC(_plist_, _hnode_, _phnode_, _pFunc_, _pparm_) \
   (PLListNodeAdd((_plist_), (HNDPLLITEM)(_hnode_), (PHNDPLLITEM)(_phnode_), \
                    (PPLLISTADDITEM)(_pFunc_), (PVOID)(_pparm_)))

// ITERATION PROCEDURE (casts parameters to the correct type)
#define PLListIterateC(_plist_, _hnode_, _flag_, _pFunc_, _pparm_, _ulMaxIterations_) \
   (PLListIterate((_plist_), (_hnode_), (_flag_), (PPLLISTITERATE)(_pFunc_), (PVOID)(_pparm_), (ULONG) (_ulMaxIterations_) ))

// other macros

#define PNODEFROMHITEM(_plist_, _hnode_) \
   ((PPLLNODE)((PBYTE)(_plist_) + (_hnode_)))

// total list size in bytes
#define PLListSize(_plist_)        ((_plist_)->cbTot)

// count of child items of _hnode_
#define PLListNodeCount(_plist_, _hnode_) \
   (PNODEFROMHITEM((_plist_), (_hnode_))->count)

// count of root items
#define PLListRootCount(_plist_)        ((_plist_)->count)

// returns data associated with a node
// works only if the item is a real node and if _hnode_ is > 0
#define PLLISTNODEDATA(_type_, _plist_, _hnode_) \
   ((_type_)((PBYTE)(_plist_) + (_hnode_) + sizeof(PLLNODE)))

// returns data associated wiht an item
// works only if the item is not a node and if _hnode_ is > 0
#define PLLISTITEMDATA(_type_, _plist_, _hitem_) \
   ((_type_)((PBYTE)(_plist_) + (_hitem_) + sizeof(PLLITEM)))

// get an item handle from an item address
#define PLLHITEM(_plist_, _pitem_) \
   ((HNDPLLITEM)((PBYTE)(_pitem_) - (PBYTE)(_plist_)))

#endif // #ifndef_PLLIST_H_
