//===========================================================================
// ApiExBase.h : generic macros and definitions
//
// 10-01-2002 * by Alessandro Felice Cantatore *
//===========================================================================

// header inclusions

#ifndef APIEX_GENERIC_H_
   #define APIEX_GENERIC_H_


/* --------------------------------------------------------------------------
 Helper for forward structure definition.
 The helper is called as:
    BUForwardDef(MYTYPE);
 The resulting code will be:
    typedef struct _MYTYPE MYTYPE;
    typedef MYTYPE * PMYTYPE;
-------------------------------------------------------------------------- */
#define BUForwardDef(_USER_DEF_TYPE_) \
   typedef struct _##_USER_DEF_TYPE_ _USER_DEF_TYPE_; \
   typedef _USER_DEF_TYPE_ * P##_USER_DEF_TYPE_


// aligns n to the next m multiple (where m is a power of 2)
#define RNDUP(n, m)   (((ULONG)(n) + (m) - 1) & ~((m) - 1))
// aligns n to the previous m multiple (where m is a power of 2)
#define RNDDWN(n, m)  ((ULONG)(n) & ~((m) - 1))

// count of array elements
#define countof(array)    (sizeof(array) / sizeof(array[0]))
//#define ciArray(array)    countof(array)

// BIT MACROS ---------------------------------------------------------------

// bit mask macros ----------------------------------------------------------

// returns the _mask_ bits of _var_
#define bitMask(_var_, _mask_)          ((_var_) & (_mask_))

// check if all bits of _mask_ are set in _var_
#define bitMaskMatch(_var_, _mask_) \
   (((_var_) & (_mask_)) == (_mask_))
//#define bitMaskIsSet(_var_, _mask_) \
//   (((_var_) & (_mask_)) == (_mask_))

// set the bits in _mask_ to 1
#define bitSetMask(_var_, _mask_) \
   ((_var_) |= (_mask_))

// set the bits in _mask_ to 0
#define bitClearMask(_var_, _mask_) \
   ((_var_) &= ~(_mask_))

// the bits defined in '_mask_' of the variable '_var_' are set to 0 or 1
// according to the value of '_val_'
#define bitSetMaskValue(_var_, _mask_, _val_) \
   ((_var_) = ((_val_)? ((_var_) | (_mask_)) : ((_var_) & ~(_mask_))))

// change the values of the bits in _mask_
#define bitToggleMask(_var_, _mask_) \
   ((_var_) = ((_var_) & ~(_mask_)) | ((_mask_) & ((_var_) ^ (_mask_))))

// set the '_mask_' bits of 'dest' to the value they have in 'src'
#define bitCopyMask(_dest_, _src_, _mask_) \
   ((_dest_) = ((_dest_) & ~(_mask_)) | ((_src_) & (_mask_)))

// compares the '_mask_' bits of 'var1' and 'var2'
// return TRUE if the bits are the same
#define bitCompMask(_var1_, _var2_, _mask_) \
   (!(((_var1_) ^ (_var2_)) & (_mask_)))

// bit index macros ---------------------------------------------------------

// create a mask (unsigned integer value) from a bit index (0 based index)
#define bitToMask(_iBit_)           (1 << (_iBit_))

// set at 1 the bit at position _iBit_ (0 based index)
#define bitSetBit(_var_, _iBit_)       bitSetMask((_var_), bitToMask(_iBit_))

// set at 0 the bit at position _iBit_ (0 based index)
#define bitClearBit(_var_, _iBit_)     bitClearMask((_var_), bitToMask(_iBit_))

// set to value (0/1) a bit
#define bitSetBitValue(_var_, _iBit_, _val_)  \
   bitSetMaskValue((_var_), bitToMask(_iBit_), (_val_))

// change the value of a bit (0 becomes 1 and viceversa)
#define bitToggleBit(_var_, _iBit_)    bitToggleMask((_var_), bitToMask(_iBit_))

// set the bit '_iBit_' of the destination to the value it has in 'src'
#define bitCopyBit(_dest_, _src_, _iBit_) \
   bitCopyMask((_dest_), (_src_), bitToMask(_iBit_))

// compares the bit '_iBit_' of '_var1_' with the corresponding bit in '_var2_'
// return TRUE if the bits are the same
#define bitCompBit(_var1_, _var2_, _iBit_) \
   bitCompMask((_var1_), (_var2_), bitToMask(_iBit_))


// DosFindFirst mask
#define FILEATTR_ANY \
   (FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY)

// Convert the address of a FILEFINDBUF3 structure to the address of a
// FILESTATUS3 structure  // 20090615 LFS not used with DataSeeker
//#define FILEFIND_TO_FILESTAT(p)  ((PFILESTATUS3)((PBYTE)(p) + sizeof(ULONG)))

// DosQuerySysInfo wrapper
_Inline ULONG SysInfo(ULONG fl) {
   DosQuerySysInfo(fl, fl, &fl, sizeof(ULONG));
   return fl;
}

// free memory and reset the address to NULL
#define freePtr(_ptr_)            free(_ptr_), _ptr_ = NULL

#endif // #ifndef APIEX_GENERIC_H_
