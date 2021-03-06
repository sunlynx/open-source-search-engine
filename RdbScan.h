// Matt Wells, copyright Sep 2000

// . used exculsively by RdbGet
// . used for scanning a key-range of records
// . does non-blocking disk reads
// . set loop to NULL to do blocking disk reads
// . we set m_errno to a positive value on error, so check it!
// . originally you could call getNextRecord() before the entire read
//   was complete. 

#ifndef GB_RDBSCAN_H
#define GB_RDBSCAN_H

#include "BigFile.h"
#include "Loop.h"
#include "RdbMap.h"
#include "RdbList.h"
#include "rdbid_t.h"

class RdbScan {
	
 public:

	// . returns false if blocked, true otherwise
	// . sets errno on error
	// . call this to start the read (sets up your key-range read too)
	// . endKey must be provided
	// . TODO: can we do multiple "simultaneous" non-blocking 
	//   reads on the same fd?
	// . we need array of files so BigFile::filecb() knows when file nuked
	bool setRead ( BigFile   *file          ,
		       int32_t       fixedDataSize ,
		       int64_t  offset        ,
		       int32_t       bytesToRead   ,
		       char      *startKey      , 
		       char      *endKey        , 
		       char       keySize       ,
		       RdbList   *list          , // we fill this up
		       void      *state         ,
		       void    (* callback ) ( void *state ) ,
		       bool       useHalfKeys   ,
		       rdbid_t    rdbId,
		       int32_t       niceness , // = MAX_NICENESS ,
		       bool       hitDisk        ); // = true );

	// RdbGet likes to get our list
	RdbList *getList ( ) { return m_rdblist; }

	// was buffer shifted down 6 bytes to turn first key into a 12 byter?
	bool wasShifted() const { return m_shifted; }
	char shiftCount() const { return m_shifted; }
	int32_t getBytesToRead() const { return m_bytesToRead; }
	int64_t getOffset() const { return m_offset; }

private:
	static void gotListWrapper0(void *state);
	void gotListWrapper();
	void gotList ( );

	// we set this list with the read buffer on read completion
	RdbList  *m_rdblist;

	// for doing non-blocking reads with BigFile::read()
	FileState m_fstate;

	// for dealing with half keys
	int32_t m_off;

	// shifting it
	char m_shifted;

	rdbid_t m_rdbId;

	// save for call to our gotListWrapper()
	char m_startKey[MAX_KEY_BYTES];
	char m_endKey  [MAX_KEY_BYTES];
	int32_t m_fixedDataSize;
	char m_useHalfKeys;
	int32_t m_bytesToRead;
	void (* m_callback ) ( void *state ) ;
	void  *m_state;

	// for sanity checking
	BigFile   *m_file;
	int64_t  m_offset;
	char       m_ks;

	bool  m_hitDisk;
};

#endif // GB_RDBSCAN_H
