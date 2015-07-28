/*******************************************************************************
Part of DataSeeker package. DataSeeker is a GUI file/text seeker for OS/2 - eCS
Copyright (C) 2014 Andreas Buchinger, 2003 Alessandro Cantatore

Usage subject to 'Modified BSD License' as described in COPYING.
*******************************************************************************/

//===========================================================================
// PLList.c : Packed Linked Lists.
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


#include "pmseek.h"

// data types ---------------------------------------------------------------


// used by the recursive iteration procedure iterateNode()
typedef struct
  {
  PPLLISTITERATE pFunc;
  PLLITERDATA iterData;
  } ITERPROCDATA, * PITERPROCDATA;


// definitions --------------------------------------------------------------

#define SLPL_DEFCBGROW      0x00100000    // 20090708 AB changed to 512kB (1MB = 0x00100000), default grow step (was 64 KB)
// 20100403 AB changed to   1MB

// macros -------------------------------------------------------------------


#define FIRSTITEM(_plist_, _pnode_) \
   ((PPLLITEM)((PBYTE)(_plist_) + (_pnode_)->offFirst))

#define NEXTITEM(_plist_, _pi_) \
   ((PPLLITEM)((PBYTE)(_plist_) + (_pi_)->offNext))

#define PREVITEM(_plist_, _pnode_) \
   ((PPLLITEM)((PBYTE)(_plist_) + (_pnode_)->offLast))

#define ITEMDATA(_pi_) \
   (((PPLLITEM)(_pi_))->isNode ? \
      (PVOID)((PPLLNODE)(_pi_) + 1) : \
      (PVOID)((PPLLITEM)(_pi_) + 1))

#define IMPLISTSHIFT         (sizeof(PLLNODE) - sizeof(PLLIST))

// prototypes ---------------------------------------------------------------

ULONG iterateNode(PPLLIST pList, PPLLNODE pNode, PITERPROCDATA pIpd, ULONG ulMaxIterations);
BOOL countItems(PPLLITERDATA p);
VOID updateImpListOffsets(PPLLIST pList, PPLLNODE pNode, ULONG cbIncr);
VOID updateExpListOffsets(PPLLIST pList, PPLLNODE pNode, ULONG cbIncr);


// implementation -----------------------------------------------------------


//===========================================================================
// Allocate storage for a new packed linked list.
// Storage allocation is increased in cbGrow steps as needed
// Parameters --------------------------------------------------------------
// ULONG cbGrow   : grow size: This must be a power of 2. If it is 0 the
//                  default size of 64 KB is used.
// Return value ------------------------------------------------------------
// PPLLIST : address of the new list, NULL in case of error.
//===========================================================================

PPLLIST PLListNew(ULONG cbGrow)
  {
  PPLLIST pList;
  if ( !cbGrow ) cbGrow = SLPL_DEFCBGROW;
  if ( NULL != (pList = malloc(RNDUP(sizeof(PLLIST), cbGrow))) )
    {
    pList->isNode = 1;
    pList->offLast = pList->cbTot = pList->offFirst = sizeof(PLLIST);
    pList->offParent = pList->count = 0;
    pList->cbGrow = cbGrow;
    TRACE4("addr=0x%X, size requested=%d, actual size=%d, pList->count=%d", pList, cbGrow, RNDUP(sizeof(PLLIST), cbGrow), pList->count );
    return pList;
    } /* endif */
  return NULL;
  }


//===========================================================================
// Duplicates a list or if pList is NULL just allocates cb bytes for a list.
// Parameters --------------------------------------------------------------
// PPLLIST pList : list to be duplicated. If this is NULL cb bytes of memory
//                 are allocated for an unitialized list.
// ULONG cb      : if cb is 0 e pList is not NULL the allocation is rounded
//                 up to the next pList->cbGrow step.
// Return value ------------------------------------------------------------
// PPLLIST : address of the new list, NULL in case of error.
//===========================================================================

PPLLIST PLListDup(PPLLIST pList, ULONG cb)
  {
  PPLLIST plist;
  if ( pList && !cb )
    cb = RNDUP(pList->cbTot, pList->cbGrow);
  TRACE1("allocate %d bytes", cb);
  if ( NULL != (plist = malloc(cb)) )
    {
    if ( pList ) memcpy(plist, pList, pList->cbTot);
    } /* endif */
  return plist;
  }


//===========================================================================
// Add a simple item (not an item which contains child items).
// Parameters --------------------------------------------------------------
// PPLLIST pList        : packed linked list
// HNDPLLITEM hNode     : handle of the parent node, NULLHANDLE if the item
//                        is a root item.
// PHNDPLLITEM phItem   : (output) handle of the new item that has been added.
// PPLLISTADDITEM pFunc : add item procedure.
//                        This procedure takes 2 parameters:
//                        - the address of the storage reserved for the item
//                          data. If this is NULL the procedure must return
//                          the size needed for the item.
//                        - user defined parameter to initialize the item.
// PVOID pParm          : user defined parameter for item initialization.
// Return value ------------------------------------------------------------
// PPLLIST : address of the packed linked list, NULL in case of error.
//           This may be different from the original 'pList' address.
//===========================================================================

PPLLIST PLListItemAdd(PPLLIST pList, HNDPLLITEM hNode, PHNDPLLITEM phItem,
                      PPLLISTADDITEM pFunc, PVOID pParm)
  {
  PPLLNODE pParent;
  ULONG cbItem;
  PPLLITEM pi;
  if ( !pList || !pFunc ) return NULL;
  // allocation needed for the current item
  cbItem = pFunc(NULL, pParm) + sizeof(PLLITEM);
  // if the current allocation size is not enough reallocate the list
  if ( RNDUP(pList->cbTot, pList->cbGrow) < RNDUP(pList->cbTot + cbItem, pList->cbGrow) )
    {
    TRACE("reallocating");
    TRACE2("pList->cbTot: %ld, cbItem: %ld", pList->cbTot, cbItem);
    pList = realloc(pList, RNDUP(pList->cbTot + cbItem, pList->cbGrow));
    TRACE("reallocating OKAY");
    }
  if ( pList )
    {
    pi = (PPLLITEM)((PBYTE)pList + pList->cbTot);
    if ( phItem ) *phItem = pList->cbTot;
    // get the address of the parent node
    pParent = PNODEFROMHITEM(pList, hNode);
    PREVITEM(pList, pParent)->offNext = pList->cbTot;
    pParent->offLast = pList->cbTot;
    pList->cbTot += cbItem;
    pi->offNext = 0;
    pi->isNode = 0;
    // set the other item data via the callback procedure
    pFunc(pi + 1, pParm);
    pParent->count++;             // update the items count
    } /* endif */
  if ( pList == NULL )
    {
    TRACE("ERROR: reallocate");
    }
  return pList;
  }


//===========================================================================
// Add a node item.
// Parameters --------------------------------------------------------------
// PPLLIST pList        : packed linked list
// HNDPLLITEM hNode     : handle of the parent node, NULLHANDLE if the item
//                        is a root item.
// PHNDPLLITEM pNode    : (output) handle of the new node that has been added.
// PPLLISTADDITEM pFunc : add item procedure.
//                        This procedure takes 2 parameters:
//                        - the address of the storage reserved for the item
//                          data. If this is NULL the procedure must return
//                          the size needed for the item.
//                        - user defined parameter to initialize the item.
// PVOID pParm          : user defined parameter for item initialization.
// Return value ------------------------------------------------------------
// PPLLIST : address of the packed linked list, NULL in case of error.
//           This may be different from the original 'pList' address.
//===========================================================================

PPLLIST PLListNodeAdd(PPLLIST pList, HNDPLLITEM hNode, PHNDPLLITEM pNode,
                      PPLLISTADDITEM pFunc, PVOID pParm)
  {
  PPLLNODE pParent, pi;
  ULONG cbItem;
  size_t stSize;
  char buffer[128];

  if ( !pList || !pFunc ) return NULL;
  // allocation needed for the current item
  cbItem = pFunc(NULL, pParm) + sizeof(PLLNODE);
  // if the current allocation size is not enough reallocate the list
  stSize = RNDUP(pList->cbTot + cbItem, pList->cbGrow);
  sprintf (buffer, "new size to allocate %ld, total: %ld, item: %ld", stSize, pList->cbTot, cbItem);
  TRACE1("%s", buffer);
  if ( (RNDUP(pList->cbTot, pList->cbGrow) >= (pList->cbTot + cbItem)) ||
       (NULL != (pList =
                 realloc(pList, stSize ))) )
    {
    TRACE1("pList after realloc: 0x%X", pList);
    pi = (PPLLNODE)((PBYTE)pList + pList->cbTot);
    if ( pNode ) *pNode = pList->cbTot;
    // get the address of the parent node
    pParent = PNODEFROMHITEM(pList, hNode);
    PREVITEM(pList, pParent)->offNext = pList->cbTot;
    pParent->offLast = pList->cbTot;
    pi->offFirst = (pList->cbTot += cbItem);
    pi->offNext = 0;
    pi->isNode = 1;
    pi->offLast = 0;
    pi->offParent = (ULONG)hNode;
    pi->count = 0;
    // set the item data via the callback procedure
    pFunc(pi + 1, pParm);
    pParent->count++;             // update the items count
    } /* endif */
  return pList;
  }


//===========================================================================
// Destroy a packed linked list.
// Parameters --------------------------------------------------------------
// PPLLIST plist : list to be destroyed
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID PLListDel(PPLLIST plist)
  {
  free(plist);
  }


//===========================================================================
// When no other items have to be added to the list, PPLListShrink()
// can be called to release the unused storage to the heap.
// Parameters --------------------------------------------------------------
// PPLLIST pList : packed linked list.
// Return value ------------------------------------------------------------
// PPLLIST : address of the reallocated list.
//===========================================================================

PPLLIST PLListShrink(PPLLIST pList)
  {

#ifdef ALTERNATE_MEM_MANAGMENT
  PPLLIST rc;       // 20091202 AB reallocating does not work !!!!!!!

  TRACE3("pList addr=0x%X, cbTot=%d, count=%d", pList, pList->cbTot, pList->count);
  rc = pList ? realloc(pList, pList->cbTot) : NULL;
  TRACE3("pList addr=0x%X, cbTot=%d, count=%d", pList, pList->cbTot, pList->count);
  return rc;
#endif  // ALTERNATE_MEM_MANAGMENT

  return pList;
  }


//===========================================================================
// Import another PLLIST as the next child item of node 'hNode'.
// Parameters --------------------------------------------------------------
// PPLLIST pList        : packed linked list
// PPLLIST pImport      : packed linked list to be imported in pList
// HNDPLLITEM hNode     : handle of the parent node, NULLHANDLE if the item
//                        is a root item.
// Return value ------------------------------------------------------------
// PPLLIST : address of the packed linked list, NULL in case of error.
//           This may be different from the original 'pList' address.
//===========================================================================

PPLLIST PLListListImport(PPLLIST pList, PPLLIST pImport, HNDPLLITEM hNode)
  {
  PPLLNODE pParent, pi;
  ULONG cb;
  INT i;
  char buffer[256];        // only for TRACE messages

  if ( !pList ) return NULL;
  // allocation needed for the current item
  cb = pImport->cbTot - sizeof(PLLIST);
  // if the current allocation size is not enough reallocate the list
  sprintf (buffer, "total (pList->cbTot): %ld, new item: %ld, pImport->cbTot: %ld sizeof(PLLIST): %ld", pList->cbTot, cb, pImport->cbTot, sizeof(PLLIST));
  TRACE1("%s", buffer);
  sprintf (buffer, "try to reallocate %ld (0x%X),cb: %ld", RNDUP(pList->cbTot + cb, pList->cbGrow), RNDUP(pList->cbTot + cb, pList->cbGrow), cb );
  TRACE1("%s", buffer);
  TRACE2("sizeof(pList) %d (0x%X)", sizeof(pList), sizeof(pList));
  TRACE2("pointer to pList before realloc: 0x%X, pImport: 0x%X", pList, pImport);
  if ( (RNDUP(pList->cbTot, pList->cbGrow) >= (pList->cbTot + cb)) ||
       (NULL != (pList =
                 realloc(pList, RNDUP(pList->cbTot + cb, pList->cbGrow)))) )
    {
    TRACE1("pList after realloc 0x%X", pList);
    pi = (PPLLNODE)((PBYTE)pList + pList->cbTot);
    // get the address of the parent node
    pParent = PNODEFROMHITEM(pList, hNode);
    TRACE1("pParent 0x%X", pParent);
    PREVITEM(pList, pParent)->offNext = pList->cbTot;
    pParent->offLast = pList->cbTot;
    // copy the data from the previous list to the current list
    memcpy(pi, (PBYTE)pImport + sizeof(PLLIST), cb);
    // update the offsets
    pParent->count += pImport->count;
    TRACE1("pParent->count %ld", pParent->count);
    for ( i = 0, cb = pList->cbTot - sizeof(PLLIST); i < pImport->count; ++i )
      {
      if ( pi->offNext )
        {
        pi->offNext += cb;
        }
      else
        {
        pParent->offLast = (HNDPLLITEM)((PBYTE)pi - (PBYTE)pList);
        } /* endif */
      // if the current item is a node
      if ( pi->isNode )
        {
        updateImpListOffsets((PPLLIST)((PBYTE)pList + cb), pi, cb);
        pi->offFirst += cb;
        pi->offLast  += cb;
        pi->offParent = hNode;
        } /* endif */
      pi = (PPLLNODE)NEXTITEM(pList, pi);
      } /* endfor */
    pList->cbTot += pImport->cbTot - sizeof(PLLIST);
    TRACE1("pList->cbTot %ld", pList->cbTot);
    } /* endif */
  TRACE1("pList after realloc 0x%X", pList);
  return pList;
  }


//===========================================================================
// Recursively update the offsets of the items imported into a list from
// another packed linked list.
// Parameters --------------------------------------------------------------
// PPLLIST pList  : packed linked list where items have been imported
// PPLLNODE pNode : node whose child item offsets must be updated
// ULONG cbIncr   : offset increment
// Return value ------------------------------------------------------------
// VOID
//===========================================================================
static
            VOID updateImpListOffsets(PPLLIST pList, PPLLNODE pNode, ULONG cbIncr)
  {
  LONG i;
  PPLLITEM pi, pnext;
  for ( i = 0, pi = FIRSTITEM(pList, pNode); i < pNode->count; ++i )
    {
    pnext = NEXTITEM(pList, pi);
    if ( pi->offNext ) pi->offNext += cbIncr;
    // if the current item is a node
    if ( pi->isNode )
      {
      updateImpListOffsets(pList, (PPLLNODE)pi, cbIncr);
      ((PPLLNODE)pi)->offFirst += cbIncr;
      ((PPLLNODE)pi)->offLast  += cbIncr;
      ((PPLLNODE)pi)->offParent += cbIncr;
      } /* endif */
    pi = pnext;
    } /* endfor */
  }


//===========================================================================
// Create a new list containing all items contained in the node 'hNode'.
// If 'hNode' is NULLHANDLE the list is duplicated.
// According to the 'inclNode' flag value the node 'hNode' is inserted
// or not in the list.
// Parameters --------------------------------------------------------------
// PPLLIST pList    : list a part of which must be exported to a new list.
// HNDPLLITEM hNode : handle of the node item
// BOOL inclNode    : include the node item in the new list
// ULONG cbGrow     : grow step of the new list (if 0 use the source list
//                    grow step).
// Return value ------------------------------------------------------------
// PPLLIST : address of the new list or NULL in case of error.
//===========================================================================

PPLLIST PLListExport(PPLLIST pList, HNDPLLITEM hNode,
                     BOOL inclNode, ULONG cbGrow)
  {
  PPLLIST pnew, p;
  ULONG cbTot;
  PPLLNODE pnode, pn;
  // check the cbGrow parameter
  if ( !cbGrow ) cbGrow = pList->cbGrow;
  // if hNode is 0 just duplicate the list
  if ( !hNode )
    {
    if ( NULL != (pnew = malloc(RNDUP(pList->cbTot, cbGrow))) )
      {
      memcpy(pnew, pList, pList->cbTot);
      pnew->cbGrow = cbGrow;
      } /* endif */
    pnew;
    } /* endif */
  // calculate the size of the allocation needed for the new list
  cbTot = sizeof(PLLIST);
  pnode = PNODEFROMHITEM(pList, hNode);
  // size of the current node (if requested)
  if ( inclNode ) cbTot += pnode->offFirst - hNode;
  // total size of the node content
  for ( pn = pnode; !pn->offNext; pn = PNODEFROMHITEM(pList, pn->offParent) ) ;
  cbTot += pn->offNext - pnode->offFirst;
  // allocate the new list
  if ( NULL != (pnew = malloc(RNDUP(cbTot, cbGrow))) )
    {
    pnew->cbTot = cbTot;
    pnew->offFirst = sizeof(PLLIST);
    pnew->offParent = 0;
    pnew->cbGrow = cbGrow;
    if ( inclNode )
      {
      memcpy((PBYTE)pnew + sizeof(PLLIST), pnode, cbTot - sizeof(PLLIST));
      pnew->count = 1;
      pnew->offLast = pnew->offFirst;
      updateExpListOffsets(pnew, (PPLLNODE)pnew,
                           hNode - sizeof(PLLIST));
      }
    else
      {
      memcpy((PBYTE)pnew + sizeof(PLLIST), FIRSTITEM(pList, pnode),
             cbTot - sizeof(PLLIST));
      pnew->count = pnode->count;
      pnew->offLast = pnode->offLast - pnode->offFirst + sizeof(PLLIST);
      updateExpListOffsets(pnew, (PPLLNODE)pnew,
                           pnode->offFirst - sizeof(PLLIST));
      } /* endif */
    } /* endif */
  return pnew;
  }


//===========================================================================
// Recursively update the offsets of the items exported to a new list from
// another packed linked list.
// Parameters --------------------------------------------------------------
// PPLLIST pList  : packed linked list where items have been imported
// PPLLNODE pNode : node whose child item offsets must be updated
// ULONG cbIncr   : offset increment
// Return value ------------------------------------------------------------
// VOID
//===========================================================================
static
            VOID updateExpListOffsets(PPLLIST pList, PPLLNODE pNode, ULONG cbIncr)
  {
  LONG i;
  PPLLITEM pi;
  for ( i = 0, pi = FIRSTITEM(pList, pNode); i < pNode->count; ++i )
    {
    if ( pi->offNext ) pi->offNext -= cbIncr;
    if ( pi->isNode )
      {
      ((PPLLNODE)pi)->offFirst -= cbIncr;
      ((PPLLNODE)pi)->offLast -= cbIncr;
      ((PPLLNODE)pi)->offParent -= cbIncr;
      updateExpListOffsets(pList, (PPLLNODE)pi, cbIncr);
      } /* endif */
    pi = NEXTITEM(pList, pi);
    } /* endfor */
  }


//===========================================================================
// Iteration procedure. For each item calls an user defined callback
// procedure passing them the current item address, the parent node address,
// the nesting level and an user defined parameter.
// If the callback procedure returns FALSE the iteration stops.
// Return the count of processed items.
// Parameters --------------------------------------------------------------
// PPLLIST pList        : packed linked list
// HNDPLLITEM hNode     : node from which the iteration starts.
//                        NULLHANDLE to start from the root.
// ULONG flag           : recursion flag
// PPLLISTITERATE pFunc : callback iteration procedure
// PVOID pParm          : user defined parameter
// ULONG ulMaxIterations: max. number of iterations, use 0 for unlimited iterations     // 20091026 AB added to prevent excessive
//   data loading into the listboxes
// Return value ------------------------------------------------------------
// ULONG count of processed items or 0 in case of broken iteration
//===========================================================================

ULONG PLListIterate(PPLLIST pList, HNDPLLITEM hNode, ULONG flag,
                    PPLLISTITERATE pFunc, PVOID pParm, ULONG ulMaxIterations)
  {
  PPLLNODE pnode, pn;
  ITERPROCDATA ipd;
  ULONG startLevel;
  ULONG citems = 0;
  PPLLITEM pi;

  //TRACE4("pList=%d, hNode=%d, flag=%d, pFunc=0x%0X", pList, hNode, flag, pFunc);
  //TRACE2("pParm=%d, ulMaxIterations=%d", pParm, ulMaxIterations);
  if ( ulMaxIterations == 0 )
    {   // set to useful limit
    ulMaxIterations = LONG_MAX;
    }

  if ( !pList ) return 0;
  pnode = pn = PNODEFROMHITEM(pList, hNode);
  ipd.pFunc = pFunc;
  ipd.iterData.pParm = pParm;
  // starting nesting level
  for ( ipd.iterData.nestLevel = 0; (ULONG)pn > (ULONG)pList; ipd.iterData.nestLevel++ )
    {
    pn = PNODEFROMHITEM(pList, pn->offParent);
    }

  if ( flag )
    {   // recursive call
    // in neither PPLLITER_NODEPRE nor PPLLITER_NODEPOST were specified
    // assume PPLLITER_NODEPRE (to process nodes before theirs descendants)
    if ( !(flag & (PPLLITER_NODEPRE | PPLLITER_NODEPOST)) ) flag |= PPLLITER_NODEPRE;

    ipd.iterData.flag = flag;
    // if the starting node is not the root and must be included
    if ( hNode && (flag & PPLLITER_INCLNODE) )
      {
      ++citems;
      if ( flag & PPLLITER_NODEPRE )
        {
        ipd.iterData.hItem = hNode;
        ipd.iterData.pData = ITEMDATA(pnode);
        // signal the callback procedure in which context it is called
        // (i.e. on the starting node and before the node descendants
        ipd.iterData.context = (PPLLITER_INCLNODE | PPLLITER_NODEPRE);
        if ( !pFunc(&ipd.iterData) ) return 0;
        } /* endif */
      // end (return) if max. number of iterations is reached
      if ( citems >= ulMaxIterations )
        {
        TRACE1("break after %d iterations (ulMaxIterations)", citems);
        return citems;
        }
      } /* endif */

    // iterate through the node descendandts
    citems = iterateNode(pList, pnode, &ipd, ulMaxIterations);

    // check if the callback procedure must be called on the starting node
    if ( bitMaskMatch(flag, (PPLLITER_INCLNODE | PPLLITER_NODEPOST)) )
      {
      ipd.iterData.hItem = hNode;
      ipd.iterData.pData = ITEMDATA(pnode);
      ipd.iterData.context = (PPLLITER_INCLNODE | PPLLITER_NODEPOST);
      if ( !pFunc(&ipd.iterData) ) return 0;
      } /* endif */
    }
  else
    {   // non recursive loop
    ipd.iterData.context = PPLLITER_NORECUR;
    //TRACE3("pList addr=0x%X, cbTot=%d, count=%d", pList, pList->cbTot, pList->count);
    //TRACE2("cbTot=0x%X, count=0x%X", pList->cbTot, pList->count);
    for ( citems = 0, pi = FIRSTITEM(pList, pnode); citems < pnode->count; ++citems )
      {
      //TRACE2("citems=%d, pnode->count=%d", citems, pnode->count );
      ipd.iterData.hItem = (HNDPLLITEM)((PBYTE)pi - (PBYTE)pList);
      ipd.iterData.pData = ITEMDATA(pi);
      if ( !pFunc(&ipd.iterData) ) return 0;
      pi = NEXTITEM(pList, pi);
      // end (return) if max. number of iterations is reached
      if ( citems >= ulMaxIterations )
        {
        TRACE1("break after %d iterations (ulMaxIterations)", citems);
        return citems;
        }
      } /* endfor */
    } /* endif */
  return citems;
  }


//===========================================================================
// Recursive procedure to iterate all children of a packed linked list node.
// Parameters --------------------------------------------------------------
// PPLLIST plist        : packed linked list
// PPLLNODE pnode       : node address
// PITERPROCDATA pipd   : iteration data structure
// ULONG ulMaxIterations: max. number of iterations, use 0 for unlimited iterations     // 20091026 AB added to prevent excessive
//   data loading into the listboxes
// Return value ------------------------------------------------------------
// ULONG count of processed items or 0 in case of broken iteration
//===========================================================================
static ULONG iterateNode(PPLLIST pList, PPLLNODE pNode, PITERPROCDATA pIpd, ULONG ulMaxIterations)
  {
  LONG tot, ci, hitem;
  PPLLITEM pi;

  if ( ulMaxIterations == 0 )
    {   // set to useful limit
    ulMaxIterations = LONG_MAX;
    }
  // check if according to the current flags must process the node now
  for ( tot = 0, hitem = pNode->offFirst, pi = FIRSTITEM(pList, pNode);
      hitem; hitem = pi->offNext, pi = NEXTITEM(pList, pi), tot++ )
    {
    // if the current item is a node
    if ( pi->isNode )
      {
      // if the item must be processed before its descendants
      if ( pIpd->iterData.flag & PPLLITER_NODEPRE )
        {
        pIpd->iterData.hItem = (HNDPLLITEM)((PBYTE)pi - (PBYTE)pList);
        pIpd->iterData.pData = (PVOID)((PPLLNODE)pi + 1);
        pIpd->iterData.context = PPLLITER_NODEPRE;
        if ( !pIpd->pFunc(&pIpd->iterData) ) return 0;
        } /* endif */
      // process the descendants
      pIpd->iterData.nestLevel++;
      if ( !( ci = iterateNode(pList, (PPLLNODE)pi, pIpd, ulMaxIterations )) ) return 0;
      tot += ci;
      pIpd->iterData.nestLevel--;
      // if the item must be processed after its descendants
      if ( pIpd->iterData.flag & PPLLITER_NODEPOST )
        {
        pIpd->iterData.hItem = (HNDPLLITEM)((PBYTE)pi - (PBYTE)pList);
        pIpd->iterData.pData = (PVOID)((PPLLNODE)pi + 1);
        pIpd->iterData.context = PPLLITER_NODEPOST;
        if ( !pIpd->pFunc(&pIpd->iterData) ) return 0;
        } /* endif */

      // end (return) if max. number of iterations is reached
      if ( tot >= ulMaxIterations )
        {
        TRACE1("break after %d iterations (ulMaxIterations)", tot);
        return tot;
        }
      }
    else
      {
      pIpd->iterData.hItem = (HNDPLLITEM)((PBYTE)pi - (PBYTE)pList);
      pIpd->iterData.pData = (PVOID)((PPLLITEM)pi + 1);
      pIpd->iterData.context = PPLLITER_NORECUR;
      if ( !pIpd->pFunc(&pIpd->iterData) ) return 0;

      // end (return) if max. number of iterations is reached
      if ( tot >= ulMaxIterations )
        {
        TRACE1("break after %d iterations (ulMaxIterations)", tot);
        return tot;
        }
      } /* endif */
    } /* endfor */
  return tot;
  }


//===========================================================================
// Returns the count of the child or descendant items of a PLLIST node.
// Parameters --------------------------------------------------------------
// PPLLIST pList      : packed linked list
// HNDPLLITEM hNode   : node handle
// BOOL bRecur        : TRUE/FALSE (include/exclude descendants)
// Return value ------------------------------------------------------------
// ULONG : item count
//===========================================================================

ULONG PLListItemsCount(PPLLIST pList, HNDPLLITEM hNode, BOOL bRecur)
  {
  PPLLNODE pnode;
  pnode = PNODEFROMHITEM(pList, hNode);
  return bRecur ?
              PLListIterateC(pList, hNode, PPLLITER_RECUR, countItems, NULL, 0) :
  pnode->count;
  }


//===========================================================================
// Callback procedure used to count the descendants of a given PLList node.
// Parameters --------------------------------------------------------------
// PPLLITERDATA p : iteration data (it is ignored)
// Return value ------------------------------------------------------------
// BOOL : always TRUE;
// VOID
//===========================================================================
static
            BOOL countItems(PPLLITERDATA p)
  {
  return TRUE;
  }


//===========================================================================
// Return the address of the data associated with the item 'hItem'.
// Parameters --------------------------------------------------------------
// PPLLIST pList    : packed linked list
// HNDPLLITEM hItem : item handle
// Return value ------------------------------------------------------------
// PVOID : address of the data associated with the item 'hItem'
//===========================================================================

PVOID PLListItemData(PPLLIST pList, HNDPLLITEM hItem)
  {
  PPLLITEM pi;
  PFOUNDFILE pff;

  TRACE1("hItem=%ld", hItem);
  if ( !pList || (hItem > PLListSize(pList)) ) return NULL;
  pi = ((PPLLITEM)((PBYTE)(pList) + (hItem)));
  pff = ITEMDATA(pi);
  TRACE2("pi=0x%X, name=%s", pi, pff->name);
  return pff;
  }


//===========================================================================
// Get the iteration data of the first item of the node p->hNode.
// Data of the next items can be retrieved via PLListItemNext().
// Parameters --------------------------------------------------------------
// PPLLIST pList : packed linked list.
// PITERITEM p   : (in/out) item iteration data.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL PLListItemFirst(PPLLIST pList, PITERITEM p)
  {
  PPLLNODE pnode;
  if ( !pList ) return FALSE;
  pnode = PNODEFROMHITEM(pList, p->hNode);
  if ( !pnode->isNode ) return FALSE;
  p->pNext = FIRSTITEM(pList, pnode);
  p->pData = ITEMDATA(p->pNext);
  p->pNext = NEXTITEM(pList, p->pNext);
  return TRUE;
  }


//===========================================================================
// Get the next item data. A previous call to PLListItemNext() is needed
// To properly initialize 'p'.
// Parameters --------------------------------------------------------------
// PPLLIST pList : packed linked list
// PITERITEM p   : (in/out) item iteration data.
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE (success/error)
//===========================================================================

BOOL PLListItemNext(PPLLIST pList, PITERITEM p)
  {
  if ( !pList || !p || !p->pNext ) return FALSE;
  p->pData = ITEMDATA(p->pNext);
  p->pNext = NEXTITEM(pList, p->pNext);
  return TRUE;
  }
