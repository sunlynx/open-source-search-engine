// Matt Wells, copyright Apr 2009

// . 2. you can also call setTitleRec() and then call getMetaList()
// . this class is used by Repair.cpp and by Msg7 (inject) and SpiderLoop.cpp
// . Msg7 and Repair.cpp and injections can also set more than just 
//   m_firstUrl, like m_content, etc. or whatever elements are known, but
//   they must also set the corresponding "valid" flags of those elements
// . both methods must yield exactly the same result, the same "meta list"
// . after setting the contained classes XmlDoc::setMetaList() makes the list
//   of rdb records to be added to all the rdbs, this is the "meta list"
// . the meta list is made by hashing all the termIds/scores into some hash
//   tables in order to accumulate scores, then the hash table are serialized
//   into the "meta list"
// . the meta list is added to all rdbs with a simple call to 
//   Msg4::addMetaList(), which is only called by Msg14 or Repair.cpp for now


#ifndef _XMLDOC_H_
#define _XMLDOC_H_

#include "Lang.h"
#include "Words.h"
#include "Bits.h"
#include "Pos.h"
#include "Phrases.h"
#include "Xml.h"
#include "SafeBuf.h"
#include "Images.h"
#include "Sections.h"
#include "Msge0.h"
#include "Msge1.h"
#include "Msg4.h"

#include "SearchInput.h"
#include "Msg40.h"
#include "Msg0.h"
#include "Msg22.h"
#include "Tagdb.h"
#include "Url.h"
#include "Linkdb.h"
#include "MsgC.h"
#include "Msg13.h"
#include "RdbList.h"
#include "SiteGetter.h"
#include "Msg20.h"
#include "Matches.h"
#include "Query.h"
#include "Title.h"
#include "Summary.h"
#include "zlib.h" // Z_OK
#include "Spider.h" // SpiderRequest/SpiderReply definitions
#include "HttpMime.h" // ET_DEFLAT
#include "Msg1.h"
#include "PingServer.h"
#include "Json.h"

#define MAXFRAGWORDS 80000

#define MAX_TAG_PAIR_HASHES 100

#include "Msg40.h"

#define POST_VECTOR_SIZE   (32*4)

#define XD_GQ_MAX_SIZE        1000

#define MAX_LINK_TEXT_LEN     512
#define MAX_SURROUNDING_TEXT_WIDTH 600
#define MAX_RSSITEM_SIZE  30000

bool setLangVec ( class Words *words , 
		  class SafeBuf *langBuf , 
		  class Sections *sections ,
		  int32_t niceness ) ;

char *getJSONFieldValue ( char *json, char *field , int32_t *valueLen ) ;

bool logQueryLogs ( );

bool getDensityRanks ( int64_t *wids , 
		       int32_t nw,
		       //int32_t wordStart , 
		       //int32_t wordEnd ,
		       int32_t hashGroup ,
		       SafeBuf *densBuf ,
		       Sections *sections ,
		       int32_t niceness );

// diversity vector
bool getDiversityVec ( class Words *words ,
		       class Phrases *phrases ,
		       class HashTableX *countTable ,
		       class SafeBuf *sbWordVec ,
		       //class SafeBuf *sbPhraseVec ,
		       int32_t niceness );

float computeSimilarity ( int32_t   *vec0 , 
			  int32_t   *vec1 ,
			  // corresponding scores vectors
			  int32_t   *s0   , 
			  int32_t   *s1   , 
			  class Query  *q    ,
			  int32_t  niceness ,
			  // only Sections::addDateBasedImpliedSections()
			  // sets this to true right now. if set to true
			  // we essentially dedup each vector, although
			  // the score is compounded into the remaining 
			  // occurence. i'm not sure if that is the right
			  // behavior though.
			  bool dedupVecs = false );

bool isSimilar_sorted ( int32_t   *vec0 , 
			int32_t   *vec1 ,
			int32_t nv0 , // how many int32_ts in vec?
			int32_t nv1 , // how many int32_ts in vec?
			// they must be this similar or more to return true
			int32_t percentSimilar,
			int32_t    niceness ) ;

int32_t getDirtyPoints ( char *s , int32_t len , int32_t niceness , char *logUrl ) ;


// tell zlib to use our malloc/free functions
int gbuncompress ( unsigned char *dest      ,
		   uint32_t *destLen   ,
		   unsigned char *source    ,
		   uint32_t  sourceLen );

int gbcompress   ( unsigned char *dest      ,
		   uint32_t *destLen   ,
		   unsigned char *source    ,
		   uint32_t  sourceLen ,
		   int32_t encoding = ET_DEFLATE);


uint32_t score8to32 ( uint8_t score8 );

// for Msg13.cpp
char getContentTypeFromContent ( char *p , int32_t niceness ) ;

// . for Msg13.cpp
// . *pend must equal \0
int32_t getContentHash32Fast ( unsigned char *p , 
			    int32_t plen ,
			    int32_t niceness ) ;

uint16_t getCharsetFast ( class HttpMime *mime, 
			  char *url ,
			  char *s , 
			  int32_t slen , 
			  int32_t niceness );

bool getWordPosVec ( Words *words ,
			    Sections *sections,
			    //int32_t wordStart,
			    //int32_t wordEnd,
			    int32_t startDist,
			    char *fragVec,
			    int32_t niceness ,
			    SafeBuf *wpos ) ;


#define ROOT_TITLE_BUF_MAX 512

class XmlDoc {

public:

	/// @warning Do NOT change. titlerec binary compatibility header

	//
	// BEGIN WHAT IS STORED IN THE TITLE REC (Titledb.h)
	//

	// headerSize = this->ptr_firstUrl - this->m_headerSize
	uint16_t  m_headerSize; 
	uint16_t  m_version;

	// these flags are used to indicate which ptr_ members are present:
	uint32_t  m_internalFlags1;
	int32_t      m_ip;
	int32_t      m_crawlDelay;

	// . use this to quickly detect if doc is unchanged
	// . we can avoid setting Xml and Words classes etc...
	int32_t      m_contentHash32;

	// this is a hash of all adjacent tag pairs for templated identificatn
	uint32_t  m_tagPairHash32;
	int32_t      m_siteNumInlinks;

	int32_t    m_reserved1;
	int32_t    m_reserved2;
	uint32_t   m_spideredTime; // time_t

	// just don't throw away any relevant SpiderRequests and we have
	// the data that m_minPubDate and m_maxPubDate provided
	//time_t    m_minPubDate;
	//time_t    m_maxPubDate;
	uint32_t  m_indexedTime; // slightly > m_spideredTime (time_t)
	uint32_t  m_reserved32;
	uint32_t  reserved3; //was: m_pubDate;    // aka m_datedbDate // time_t
	//time_t  m_nextSpiderTime;
	uint32_t    m_firstIndexedDate; // time_t
	uint32_t    m_outlinksAddedDate; // time_t

	uint16_t  m_charset; // the ORIGINAL charset, we are always utf8!
	uint16_t  m_countryId;

	int32_t      m_reserved3;

	uint8_t   m_metaListCheckSum8; // bring it back!!
	char      m_reserved3b;
	uint16_t  m_bodyStartPos;
	uint16_t  m_reserved5;

	uint16_t  m_unused0; //was: m_diffbotJSONCount

	int16_t   m_httpStatus; // -1 if not found (empty http reply)
	
	int8_t    m_hopCount;
	uint8_t   m_langId;
	uint8_t   m_rootLangId;
	uint8_t   m_contentType;


	// bit flags
	uint16_t  m_isRSS:1;
	uint16_t  m_isPermalink:1;
	uint16_t  m_isAdult:1;
	uint16_t  m_wasContentInjected:1;//eliminateMenus:1;
	uint16_t  m_spiderLinks:1;
	uint16_t  m_isContentTruncated:1;
	uint16_t  m_isLinkSpam:1;
	uint16_t  m_reserved796:1;
	uint16_t  m_reserved797:1;
	uint16_t  m_reserved798:1;
	uint16_t  m_reserved799:1;
	uint16_t  m_isSiteRoot:1;

	uint16_t  m_reserved800:1; //was:m_isDiffbotJSONObject
	uint16_t  m_reserved801:1; //was:m_sentToDiffbot
	uint16_t  m_reserved802:1; //was:m_gotDiffbotSuccessfulReply
	uint16_t  m_useTimeAxis:1; // m_reserved804:1;
	uint16_t  m_reserved805:1;
	uint16_t  m_reserved806:1;
	uint16_t  m_reserved807:1;
	uint16_t  m_reserved808:1;
	uint16_t  m_reserved809:1;
	uint16_t  m_reserved810:1;
	uint16_t  m_reserved811:1;
	uint16_t  m_reserved812:1;
	uint16_t  m_reserved813:1;
	uint16_t  m_reserved814:1;
	uint16_t  m_reserved815:1;
	uint16_t  m_reserved816:1;

	//end of titlerec binary compatibility header

	/// @warning Do NOT change the structure of the following until m_dummyEnd.
	/// check in XmlDoc::set2 (sanity check. must match exactly)

	char      *ptr_firstUrl;
	char      *ptr_redirUrl;
	char      *ptr_rootTitleBuf;
	int32_t      *ptr_unused12;
	int32_t      *ptr_unused13;
	void      *ptr_unused8;
	int64_t *ptr_unused10;
	rscore_t  *ptr_unused11;
	char      *ptr_imageData;
	int32_t      *ptr_unused6;
	int32_t      *ptr_unused7;
	char      *ptr_unused1;
	char      *ptr_unused2;
	char      *ptr_unused3;
	char      *ptr_utf8Content;
	char      *ptr_unused5;

	// do not let SiteGetter change this when we re-parse!
	char      *ptr_site;
	LinkInfo  *ptr_linkInfo1;
	char      *ptr_linkdbData;
	char      *ptr_sectiondbData;
	char      *ptr_tagRecData;
	LinkInfo  *ptr_unused9;

	int32_t       size_firstUrl;
	int32_t       size_redirUrl;
	int32_t       size_rootTitleBuf;
	int32_t       size_unused12;
	int32_t       size_unused13;
	int32_t       size_unused8;
	int32_t       size_unused10;
	int32_t       size_unused11;
	int32_t       size_imageData;
	int32_t       size_unused6;
	int32_t       size_unused7;
	int32_t       size_unused1;
	int32_t       size_unused2;
	int32_t       size_unused3;
	int32_t       size_utf8Content;
	int32_t       size_unused5;
	int32_t       size_site;
	int32_t       size_linkInfo1;
	int32_t       size_linkdbData;
	int32_t       size_sectiondbData;
	int32_t       size_tagRecData;
	int32_t       size_unused9;

	char      m_dummyEnd;

	//
	// END WHAT IS STORED IN THE TITLE REC (Titledb.h)
	//

	char		*ptr_scheme;	
	int32_t		size_scheme;	


 public:


	void print   ( );

	bool set2 ( char *titleRec,
		    int32_t maxSize, 
		    char *coll,
		    class SafeBuf *p,
		    int32_t niceness ,
		    class SpiderRequest *sreq = NULL );

	// . since being set from a docId, we will load the old title rec
	//   and use that!
	// . used by PageGet.cpp
	bool set3 ( int64_t  docId       , 
		    const char   *coll        ,
		    int32_t       niceness    );

	bool set4 ( class SpiderRequest *sreq  , 
		    key_t           *doledbKey ,
		    char            *coll      , 
		    class SafeBuf   *pbuf      , 
		    int32_t          niceness  ,
		    char            *utf8Content = NULL ,
		    bool             deleteFromIndex = false ,
		    int32_t             forcedIp = 0 ,
		    uint8_t          contentType = CT_HTML ,
		    uint32_t           spideredTime = 0 , // time_t
		    bool             contentHasMime = false );

	// we now call this right away rather than at download time!
	int32_t getSpideredTime();

	// time right before adding the termlists to the index, etc.
	// whereas spider time is the download time
	int32_t getIndexedTime();

	// another entry point, like set3() kinda
	bool loadFromOldTitleRec ();

	XmlDoc() ; 
	~XmlDoc() ; 
	void nukeDoc ( class XmlDoc *);
	void reset ( ) ;
	bool setFirstUrl ( char *u , bool addWWW , Url *base = NULL ) ;
	bool setRedirUrl ( char *u , bool addWWW ) ;
	void setStatus ( char *s ) ;
	void setCallback ( void *state, void (*callback) (void *state) ) ;
	void setCallback ( void *state, bool (*callback) (void *state) ) ;
	void getRevisedSpiderRequest ( class SpiderRequest *revisedReq );
	void getRebuiltSpiderRequest ( class SpiderRequest *sreq ) ;
	bool indexDoc ( );
	bool indexDoc2 ( );

	key_t *getTitleRecKey() ;
	//char *getSkipIndexing ( );
	char *prepareToMakeTitleRec ( ) ;
	// store TitleRec into "buf" so it can be added to metalist
	bool setTitleRecBuf ( SafeBuf *buf , int64_t docId, int64_t uh48 );
	// sets m_titleRecBuf/m_titleRecBufValid/m_titleRecKey[Valid]
	SafeBuf *getTitleRecBuf ( );
	SafeBuf *getSpiderStatusDocMetaList ( class SpiderReply *reply ,
					      bool forDelete ) ;
	SafeBuf *getSpiderStatusDocMetaList2 ( class SpiderReply *reply ) ;
	bool setSpiderStatusDocMetaList ( SafeBuf *jd , int64_t ssDocId ) ;
	SafeBuf m_spiderStatusDocMetaList;
	char *getIsAdult ( ) ;

	int32_t getOutlinkAge ( int32_t outlinkNum ) ;
	char *getIsPermalink ( ) ;
	char *getIsUrlPermalinkFormat ( ) ;
	char *getIsRSS ( ) ;
	char *getIsSiteMap ( ) ;
	class Xml *getXml ( ) ;
	uint8_t *getLangVector ( ) ;	
	uint8_t *getLangId ( ) ;
	char computeLangId ( Sections *sections ,Words *words , char *lv ) ;
	class Words *getWords ( ) ;
	class Bits *getBits ( ) ;
	class Bits *getBitsForSummary ( ) ;
	class Pos *getPos ( );
	class Phrases *getPhrases ( ) ;
	class Sections *getSections ( ) ;
	int32_t *getLinkSiteHashes ( );
	class Links *getLinks ( bool doQuickSet = false ) ;
	class HashTableX *getCountTable ( ) ;
	bool hashString_ct ( class HashTableX *ht, char *s , int32_t slen ) ;
	uint8_t *getSummaryLangId ( ) ;
	int32_t *getSummaryVector ( ) ;
	int32_t *getPageSampleVector ( ) ;
	int32_t *getPostLinkTextVector ( int32_t linkNode ) ;
	int32_t computeVector ( class Words *words, uint32_t *vec , int32_t start = 0 , int32_t end = -1 );
	float *getTagSimilarity ( class XmlDoc *xd2 ) ;
	float *getPageSimilarity ( class XmlDoc *xd2 ) ;
	float *getPercentChanged ( );
	uint64_t *getFuzzyDupHash ( );
	int64_t *getExactContentHash64();
	class RdbList *getDupList ( ) ;
	class RdbList *getLikedbListForReq ( );
	class RdbList *getLikedbListForIndexing ( );
	char *getIsDup ( ) ;
	char *getMetaDescription( int32_t *mdlen ) ;
	char *getMetaSummary ( int32_t *mslen ) ;
	char *getMetaKeywords( int32_t *mklen ) ;
	char *getMetaGeoPlacename( int32_t *mgplen );
	
	int32_t *getSiteSpiderQuota ( ) ;
	class Url *getCurrentUrl ( ) ;
	class Url *getFirstUrl() ;
	int64_t getFirstUrlHash48();
	int64_t getFirstUrlHash64();
	class Url **getLastRedirUrl() ;
	class Url **getRedirUrl() ;
	class Url **getMetaRedirUrl() ;
	class Url **getCanonicalRedirUrl ( ) ;
	int32_t *getFirstIndexedDate ( ) ;
	int32_t *getOutlinksAddedDate ( ) ;
	//int32_t *getNumBannedOutlinks ( ) ;
	uint16_t *getCountryId ( ) ;
	class XmlDoc **getOldXmlDoc ( ) ;
	//bool isRobotsTxtFile ( char *url , int32_t urlLen ) ;
	class XmlDoc **getExtraDoc ( char *url , int32_t maxCacheAge = 0 ) ;
	bool getIsPageParser ( ) ;
	class XmlDoc **getRootXmlDoc ( int32_t maxCacheAge = 0 ) ;
	//class XmlDoc **getGatewayXmlDoc ( ) ;
	// . returns false if blocked, true otherwise.
	// . returns true and sets g_errno on error
	//bool setFromOldTitleRec ( ) ;
	//RdbList *getOldMetaList ( ) ;
	char **getOldTitleRec ( );
	uint8_t *getRootLangId ();
	//bool *updateRootLangId ( );
	char **getRootTitleRec ( ) ;
	int64_t *getAvailDocIdOnly ( int64_t preferredDocId ) ;
	int64_t *getDocId ( ) ;
	char *getIsIndexed ( ) ;
	class TagRec *getTagRec ( ) ;
	// non-dup/nondup addresses only
	int32_t *getFirstIp ( ) ;
	bool *updateFirstIp ( ) ;
	//int32_t *getSiteNumInlinksUniqueIp ( ) ;
	//int32_t *getSiteNumInlinksUniqueCBlock ( ) ;
	//int32_t *getSiteNumInlinksTotal ( );
	//int32_t *getSiteNumInlinksFresh ( ) ;
	//int32_t *getSitePop ( ) ;
	uint8_t *getSiteNumInlinks8 () ;
	int32_t *getSiteNumInlinks ( ) ;
	class LinkInfo *getSiteLinkInfo() ;
	int32_t *getIp ( ) ;
	int32_t *gotIp ( bool save ) ;
	bool *getIsAllowed ( ) ;
	int32_t *getFinalCrawlDelay();
	int32_t      m_finalCrawlDelay;
	//int32_t getTryAgainTimeDelta() { 
	//	if ( ! m_tryAgainTimeDeltaValid ) { char *xx=NULL;*xx=0;}
	//	return m_tryAgainTimeDelta;
	//};
	char *getIsWWWDup ( ) ;
	class LinkInfo *getLinkInfo1 ( ) ;
	char *getSite ( ) ;
	char *getScheme ( ) ;	
	
	void  gotSite ( ) ;
	int64_t *getSiteHash64 ( ) ;
	int32_t *getSiteHash32 ( ) ;
	char **getHttpReply ( ) ;
	char **getHttpReply2 ( ) ;
	char **gotHttpReply ( ) ;
	char *getIsContentTruncated ( );
	int32_t *getDownloadStatus ( ) ;
	int64_t *getDownloadEndTime ( ) ;
	int16_t *getHttpStatus ( );
	char waitForTimeSync ( ) ;
	bool m_alreadyRegistered;
	class HttpMime *getMime () ;
	char **getContent ( ) ;
	uint8_t *getContentType ( ) ;
	uint16_t *getCharset ( ) ;
	char **getFilteredContent ( ) ;
	void filterStart_r ( bool amThread ) ;
	char **getRawUtf8Content ( ) ;
	char **getExpandedUtf8Content ( ) ;
	char **getUtf8Content ( ) ;
	// we download large files to a file on disk, like warcs and arcs
	int32_t *getContentHash32 ( ) ;
	int32_t *getContentHashJson32 ( ) ;
	//int32_t *getTagHash32 ( ) ;
	int32_t     *getTagPairHashVector ( ) ;
	uint32_t *getTagPairHash32 ( ) ;
	int32_t getHostHash32a ( ) ;
	int32_t getHostHash32b ( ) ;
	int32_t getDomHash32 ( );
	char **getThumbnailData();
	class Images *getImages ( ) ;
	int8_t *getNextSpiderPriority ( ) ;
	int32_t *getPriorityQueueNum ( ) ;
	class TagRec ***getOutlinkTagRecVector () ;
	char *hasNoIndexMetaTag();
	char *hasFakeIpsMetaTag ( );
	int32_t **getOutlinkFirstIpVector () ;
	//char **getOutlinkIsIndexedVector () ;
	int32_t *getRegExpNum ( int32_t outlinkNum ) ;
	int32_t *getRegExpNum2 ( int32_t outlinkNum ) ;
	char *getIsSiteRoot ( ) ;
	bool getIsOutlinkSiteRoot ( char *u , class TagRec *gr ) ;
	int8_t *getHopCount ( ) ;
	//int8_t *getOutlinkHopCountVector ( ) ;
	char *getSpiderLinks ( ) ;
	int32_t *getNextSpiderTime ( ) ;
	//char *getIsSpam() ;
	char *getIsFiltered ();
	bool getIsInjecting();
	int32_t *getSpiderPriority ( ) ;
	int32_t *getIndexCode ( ) ;
	int32_t *getIndexCode2 ( ) ;
	SafeBuf *getNewTagBuf ( ) ;

	char *updateTagdb ( ) ;
	bool logIt ( class SafeBuf *bb = NULL ) ;
	bool m_doConsistencyTesting;
	bool doConsistencyTest ( bool forceTest ) ;
	int32_t printMetaList ( ) ;
	void printMetaList ( char *metaList , char *metaListEnd ,
			     class SafeBuf *pbuf );
	bool verifyMetaList ( char *p , char *pend , bool forDelete ) ;
	bool hashMetaList ( class HashTableX *ht        ,
			    char       *p         ,
			    char       *pend      ,
			    bool        checkList ) ;

	char *getMetaList ( bool forDelete = false );

	uint64_t m_downloadStartTime;
	//uint64_t m_downloadEndTime;

	uint64_t m_ipStartTime;
	uint64_t m_ipEndTime;

	bool m_updatedMetaData;

	void copyFromOldDoc ( class XmlDoc *od ) ;

	class SpiderReply *getFakeSpiderReply ( );

	// we add a SpiderReply to spiderdb when done spidering, even if
	// m_indexCode or g_errno was set!
	class SpiderReply *getNewSpiderReply ( );


	SpiderRequest **getRedirSpiderRequest ( );
	SpiderRequest m_redirSpiderRequest;
	SpiderRequest *m_redirSpiderRequestPtr;


	void  setSpiderReqForMsg20 ( class SpiderRequest *sreq , 
				     class SpiderReply   *srep );


	char *addOutlinkSpiderRecsToMetaList ( );

	int32_t getSiteRank ();
	bool addTable144 ( class HashTableX *tt1 , 
			   int64_t docId ,
			   class SafeBuf *buf = NULL );

	bool addTable224 ( HashTableX *tt1 ) ;

	bool addTable128 ( class HashTableX *tt1     , // T <key128_t,char>*tt1
                           uint8_t     rdbId   ,
			   bool        forDelete ) ;

	bool hashNoSplit ( class HashTableX *tt ) ;
	char *hashAll ( class HashTableX *table ) ;
	int32_t getBoostFromSiteNumInlinks ( int32_t inlinks ) ;
	bool hashSpiderReply (class SpiderReply *reply ,class HashTableX *tt) ;
	bool hashMetaTags ( class HashTableX *table ) ;
	bool hashIsClean ( class HashTableX *table ) ;
	bool hashZipCodes ( class HashTableX *table ) ;
	bool hashMetaZip ( class HashTableX *table ) ;
	bool hashContentType ( class HashTableX *table ) ;
	
	bool hashLinks ( class HashTableX *table ) ;
	bool getUseTimeAxis ( ) ;
	SafeBuf *getTimeAxisUrl ( );
	bool hashUrl ( class HashTableX *table, bool urlOnly );
	bool hashDateNumbers ( class HashTableX *tt );
	bool hashSections ( class HashTableX *table ) ;
	bool hashIncomingLinkText( class HashTableX *table, bool hashAnomalies, bool hashNonAnomalies );
	bool hashLinksForLinkdb ( class HashTableX *table ) ;
	bool hashNeighborhoods ( class HashTableX *table ) ;
	bool hashRSSInfo ( class HashTableX *table ) ;
	bool hashRSSTerm ( class HashTableX *table , bool inRSS ) ;
	bool hashTitle ( class HashTableX *table );
	bool hashBody2 ( class HashTableX *table );
	bool hashMetaKeywords ( class HashTableX *table );
	bool hashMetaGeoPlacename( class HashTableX *table );
	bool hashMetaSummary ( class HashTableX *table );
	bool hashLanguage ( class HashTableX *table ) ;
	bool hashLanguageString ( class HashTableX *table ) ;
	bool hashCountry ( class HashTableX *table ) ;
	bool hashSiteNumInlinks ( class HashTableX *table ) ;
	bool hashCharset ( class HashTableX *table ) ;
	bool hashTagRec ( class HashTableX *table ) ;
	bool hashPermalink ( class HashTableX *table ) ;
	bool hashVectors(class HashTableX *table ) ;
	
	class Url *getBaseUrl ( ) ;
	bool hashIsAdult    ( class HashTableX *table ) ;

	void set20 ( Msg20Request *req ) ;
	class Msg20Reply *getMsg20Reply ( ) ;
	class MatchOffsets *getMatchOffsets () ;
	Query *getQuery() ;
	Matches *getMatches () ;
	char *getDescriptionBuf ( char *displayMetas , int32_t *dlen ) ;
	SafeBuf *getHeaderTagBuf();
	class Title *getTitle ();
	class Summary *getSummary () ;
	char *getHighlightedSummary ();
	char *getIsNoArchive ( ) ;
	int32_t *getUrlFilterNum();
	char *getIsLinkSpam ( ) ;
	char *getIsHijacked();
	char *getIsErrorPage ( ) ;
	char* matchErrorMsg(char* p, char* pend );

	bool hashWords( class HashInfo *hi );
	bool hashSingleTerm( int64_t termId, class HashInfo *hi );
	bool hashSingleTerm( char *s, int32_t slen, class HashInfo *hi );
	bool hashString( class HashTableX *ht, class Bits *bits, char *s, int32_t slen );
	bool hashString( char *s, int32_t slen, class HashInfo *hi );
	bool hashString( char *s, class HashInfo *hi );

	bool hashWords3( class HashInfo *hi, class Words *words, class Phrases *phrases, class Synonyms *synonyms,
					 class Sections *sections, class HashTableX *countTable, char *fragVec, char *wordSpamVec,
					 char *langVec, char docLangId, class SafeBuf *pbuf, class HashTableX *wts,
					 class SafeBuf *wbuf, int32_t niceness );

	bool hashString3( char *s, int32_t slen, class HashInfo *hi, class HashTableX *countTable,
					  class SafeBuf *pbuf, class HashTableX *wts, class SafeBuf *wbuf, int32_t version,
					  int32_t siteNumInlinks, int32_t niceness );

	// gbfieldmatch:
	bool hashFieldMatchTerm ( char *val, int32_t vlen, class HashInfo *hi);

	bool hashNumberForSorting( char *beginBuf ,
			  char *buf , 
			  int32_t bufLen , 
			  class HashInfo *hi ) ;

	bool hashNumberForSortingAsFloat ( float f , 
			   class HashInfo *hi ,
			   char *gbsortByStr ) ;

	bool hashNumberForSortingAsInt32 ( int32_t x,
			   class HashInfo *hi ,
			   char *gbsortByStr ) ;

	// print out for PageTitledb.cpp and PageParser.cpp
	bool printDoc ( class SafeBuf *pbuf );
	bool printMenu ( class SafeBuf *pbuf );
	bool printDocForProCog ( class SafeBuf *sb , HttpRequest *hr ) ;
	bool printGeneralInfo ( class SafeBuf *sb , HttpRequest *hr ) ;
	bool printRainbowSections ( class SafeBuf *sb , HttpRequest *hr );
	bool printSiteInlinks ( class SafeBuf *sb , HttpRequest *hr );
	bool printPageInlinks ( class SafeBuf *sb , HttpRequest *hr );
	bool printTermList ( class SafeBuf *sb , HttpRequest *hr );
	bool printSpiderStats ( class SafeBuf *sb , HttpRequest *hr );
	bool printCachedPage ( class SafeBuf *sb , HttpRequest *hr );

	char **getTitleBuf             ( );
	char **getRootTitleBuf         ( );
	char **getFilteredRootTitleBuf ( );

 public:

	// stuff set from the key of the titleRec, above the compression area
	int64_t m_docId;

	char     *m_ubuf;
	int32_t      m_ubufSize;
	int32_t      m_ubufAlloc;

	// private:

	// we we started spidering it, in milliseconds since the epoch
	int64_t    m_startTime;
	int64_t    m_injectStartTime;

	class XmlDoc *m_prevInject;
	class XmlDoc *m_nextInject;

	// when set() was called by Msg20.cpp so we can time how long it took
	// to generate the summary
	int64_t    m_setTime;
	int64_t    m_cpuSummaryStartTime;

	// . these should all be set using set*() function calls so their
	//   individual validity flags can bet set to true, and successive
	//   calls to their corresponding get*() functions will not core
	// . these particular guys are set immediately on set(char *titleRec)

	Url        m_redirUrl;
	Url       *m_redirUrlPtr;
	Url       *m_lastRedirUrlPtr;
	SafeBuf    m_redirCookieBuf;
	Url        m_metaRedirUrl;
	Url       *m_metaRedirUrlPtr;
	Url        m_canonicalRedirUrl;
	Url       *m_canonicalRedirUrlPtr;
	int32_t       m_redirError;
	char       m_allowSimplifiedRedirs;
	Url        m_firstUrl;
	int64_t  m_firstUrlHash48;
	int64_t  m_firstUrlHash64;
	Url        m_currentUrl;

	CollectionRec *m_lastcr;
	collnum_t      m_collnum;
	int32_t           m_lastCollRecResetCount;
	class CollectionRec *getCollRec ( ) ;
	bool setCollNum ( const char *coll ) ;


	char      *m_content;
	int32_t       m_contentLen;

	char *m_metaList;
	int32_t  m_metaListSize;

	int32_t m_addedSpiderRequestSize;
	int32_t m_addedSpiderReplySize;
	int32_t m_addedStatusDocSize;

	SafeBuf  m_metaList2;
	SafeBuf  m_zbuf;
	SafeBuf  m_kbuf;

	// used by msg7 to store udp slot
	class UdpSlot *m_injectionSlot;

	// . same thing, a little more complicated
	// . these classes are only set on demand
	Xml        m_xml;
	Links      m_links;
	Words      m_words;
	Bits       m_bits;
	Bits       m_bits2;
	Pos        m_pos;
	Phrases    m_phrases;
	SafeBuf    m_synBuf;
	Sections   m_sections;

	// . for rebuild logging of what's changed
	// . Repair.cpp sets these based on titlerec
	char m_logLangId;
	int32_t m_logSiteNumInlinks;

	SafeBuf m_tmpBuf2;

	SafeBuf m_timeAxisUrl;

	Images     m_images;
	HashTableX m_countTable;
	HttpMime   m_mime;
	TagRec     m_tagRec;
	SafeBuf    m_tagRecBuf;
	SafeBuf    m_newTagBuf;
	SafeBuf    m_fragBuf;
	SafeBuf    m_wordSpamBuf;
	SafeBuf    m_finalSummaryBuf;
	int32_t       m_firstIp;

	class SafeBuf     *m_savedSb;
	class HttpRequest *m_savedHr;

	// validity flags. on reset() all these are set to false.
	char     m_VALIDSTART;
	// DO NOT add validity flags above this line!
	char     m_metaListValid;
	char     m_addedSpiderRequestSizeValid;
	char     m_addedSpiderReplySizeValid;
	char     m_addedStatusDocSizeValid;
	char     m_downloadStartTimeValid;
	char     m_siteValid;
	char     m_startTimeValid;
	char     m_currentUrlValid;
	char     m_useTimeAxisValid;
	char     m_timeAxisUrlValid;
	char     m_firstUrlValid;
	char     m_firstUrlHash48Valid;
	char     m_firstUrlHash64Valid;
	char     m_lastUrlValid;
	char     m_docIdValid;
	char     m_availDocIdValid;
	char     m_tagRecValid;
	char     m_robotsTxtLenValid;
	char     m_tagRecDataValid;
	char     m_newTagBufValid;
	char     m_rootTitleBufValid;
	char     m_filteredRootTitleBufValid;
	char     m_titleBufValid;
	char     m_fragBufValid;
	char     m_wordSpamBufValid;
	char     m_finalSummaryBufValid;
	char     m_redirSpiderRequestValid;

	char     m_hopCountValid;
	char     m_isInjectingValid;
	char     m_isImportingValid;
	char     m_metaListCheckSum8Valid;
	char     m_contentValid;
	char     m_filteredContentValid;
	char     m_charsetValid;
	char     m_langVectorValid;
	char     m_langIdValid;
	char     m_rootLangIdValid;
	char     m_datedbDateValid;
	char     m_isRSSValid;
	char     m_isSiteMapValid;
	char     m_spiderLinksArgValid;
	char     m_isContentTruncatedValid;
	char     m_xmlValid;
	char     m_linksValid;
	char     m_wordsValid;
	char     m_bitsValid;
	char     m_bits2Valid;
	char     m_posValid;
	char     m_isUrlBadYearValid;
	char     m_phrasesValid;
	char     m_sectionsValid;
	char     m_subSentsValid;

	char     m_imageDataValid;
	char     m_imagesValid;
	char     m_msge0Valid;
	char     m_msge1Valid;
	char     m_sreqValid;
	char     m_srepValid;

	bool m_ipValid;
	bool m_firstIpValid;
	bool m_spideredTimeValid;
	bool m_indexedTimeValid;
	bool m_firstIndexedValid;
	bool m_isInIndexValid;
	bool m_wasInIndexValid;
	bool m_outlinksAddedDateValid;
	bool m_countryIdValid;
	bool m_bodyStartPosValid;

	bool m_httpStatusValid;
	bool m_crawlDelayValid;
	bool m_finalCrawlDelayValid;
	bool m_titleRecKeyValid;
	bool m_versionValid;
	bool m_rawUtf8ContentValid;
	bool m_expandedUtf8ContentValid;
	bool m_utf8ContentValid;
	bool m_isAllowedValid;
	bool m_redirUrlValid;
	bool m_redirCookieBufValid;
	bool m_metaRedirUrlValid;
	bool m_canonicalRedirUrlValid;
	bool m_statusMsgValid;
	bool m_mimeValid;
	bool m_pubDateValid;
	bool m_hostHash32aValid;
	bool m_hostHash32bValid;
	bool m_indexCodeValid;
	bool m_priorityValid;
	bool m_downloadStatusValid;
	bool m_downloadEndTimeValid;
	bool m_redirErrorValid;
	bool m_domHash32Valid;
	bool m_contentHash32Valid;
	bool m_tagPairHash32Valid;

	bool m_spiderLinksValid;
	bool m_firstIndexedDateValid;
	bool m_isPermalinkValid;

	bool m_isAdultValid;
	bool m_urlPubDateValid;
	bool m_isUrlPermalinkFormatValid;
	bool m_percentChangedValid;
	bool m_unchangedValid;
	bool m_countTableValid;
	bool m_summaryLangIdValid;
	bool m_tagPairHashVecValid;
	bool m_summaryVecValid;
	bool m_titleVecValid;
	bool m_pageSampleVecValid;
	bool m_postVecValid;
	bool m_dupListValid;
	bool m_likedbListValid;
	bool m_isDupValid;
	bool m_metaDescValid;
	bool m_metaSummaryValid;
	bool m_metaKeywordsValid;
	bool m_metaGeoPlacenameValid;
	bool m_siteSpiderQuotaValid;
	bool m_oldDocValid;
	bool m_extraDocValid;
	bool m_rootDocValid;
	bool m_oldMetaListValid;
	bool m_oldTitleRecValid;
	bool m_rootTitleRecValid;
	bool m_isIndexedValid;
	bool m_siteNumInlinksValid;
	bool m_siteNumInlinks8Valid;
	bool m_siteLinkInfoValid;
	bool m_isWWWDupValid;
	bool m_linkInfo1Valid;
	bool m_linkSiteHashesValid;
	bool m_siteHash64Valid;
	bool m_siteHash32Valid;
	bool m_httpReplyValid;
	bool m_contentTypeValid;
	bool m_priorityQueueNumValid;
	bool m_outlinkTagRecVectorValid;
	bool m_outlinkIpVectorValid;
	bool m_hasNoIndexMetaTagValid;
	bool m_hasUseFakeIpsMetaTagValid;
	bool m_outlinkIsIndexedVectorValid;
	bool m_isSiteRootValid;
	bool m_wasContentInjectedValid;
	bool m_outlinkHopCountVectorValid;
	bool m_isFilteredValid;
	bool m_urlFilterNumValid;
	bool m_numOutlinksAddedValid;
	bool m_baseUrlValid;
	bool m_replyValid;
	bool m_crawlInfoValid;
	bool m_isPageParserValid;
	bool m_imageUrlValid;
	bool m_imageUrl2Valid;
	bool m_matchOffsetsValid;
	bool m_queryValid;
	bool m_matchesValid;
	bool m_dbufValid;
	bool m_titleValid;
	bool m_htbValid;
	bool m_collnumValid;
	bool m_summaryValid;
	bool m_gsbufValid;
	bool m_spiderStatusDocMetaListValid;
	bool m_isCompromisedValid;
	bool m_isNoArchiveValid;
	bool m_titleRecBufValid;
	bool m_isLinkSpamValid;
	bool m_isErrorPageValid;
	bool m_isHijackedValid;
	bool m_dupHashValid;
	bool m_exactContentHash64Valid;
	bool m_looseContentHash64Valid;
	bool m_jpValid;

	char m_isSiteMap;

	// shadows
	char m_isRSS2;
	char m_isPermalink2;
	char m_isAdult2;
        char m_spiderLinks2;
	char m_isContentTruncated2;
	char m_isLinkSpam2;
	char m_isSiteRoot2;

	// DO NOT add validity flags below this line!
	char     m_VALIDEND;


	bool m_printedMenu;
	int32_t m_urlPubDate;
	char m_isUrlPermalinkFormat;
	uint8_t m_summaryLangId;
	int32_t m_tagPairHashVec[MAX_TAG_PAIR_HASHES];
	int32_t m_tagPairHashVecSize;
	int32_t m_summaryVec [SAMPLE_VECTOR_SIZE/4];
	int32_t m_summaryVecSize;
	int32_t m_titleVec [SAMPLE_VECTOR_SIZE/4];
	int32_t m_titleVecSize;
	int32_t m_pageSampleVec[SAMPLE_VECTOR_SIZE/4];
	int32_t m_pageSampleVecSize;
	int32_t m_postVec[POST_VECTOR_SIZE/4];
	int32_t m_postVecSize;
	float m_tagSimilarity;
	float m_pageSimilarity;
	float m_percentChanged;
	bool  m_unchanged;
	// what docids are similar to us? docids are in this list
	RdbList m_dupList;
	RdbList m_likedbList;
	uint64_t m_dupHash;
	int64_t m_exactContentHash64;
	int64_t m_looseContentHash64;
	Msg0 m_msg0;
	Msg5 m_msg5;
	char m_isDup;
	int64_t m_docIdWeAreADupOf;
	int32_t m_ei;
	int32_t m_lastLaunch;
	Msg22Request m_msg22Request;
	Msg22Request m_msg22Requestc;
	Msg22 m_msg22a;
	Msg22 m_msg22b;
	Msg22 m_msg22c;
	Msg22 m_msg22d;
	Msg22 m_msg22e;
	Msg22 m_msg22f;
	// these now reference directly into the html src so our 
	// WordPosInfo::m_wordPtr algo works in seo.cpp
	char *m_metaDesc;
	int32_t  m_metaDescLen;
	char *m_metaSummary;
	int32_t  m_metaSummaryLen;
	char *m_metaKeywords;
	int32_t  m_metaKeywordsLen;
	
	char *m_metaGeoPlacename;
	int32_t  m_metaGeoPlacenameLen;
	
	
	int32_t  m_siteSpiderQuota;
	class XmlDoc *m_oldDoc;
	class XmlDoc *m_extraDoc;
	class XmlDoc *m_rootDoc;
	RdbList m_oldMetaList;
	char   *m_oldTitleRec;
	int32_t    m_oldTitleRecSize;
	char   *m_rootTitleRec;
	int32_t    m_rootTitleRecSize;
	char    m_isIndexed;

	// confusing, i know! these are used exclsusively by
	// getNewSpiderReply() for now
	char m_isInIndex;
	char m_wasInIndex;

	Msg8a   m_msg8a;
	char   *m_tagdbColl;
	int32_t    m_tagdbCollLen;

	Url   m_extraUrl;
	uint8_t m_siteNumInlinks8;
	LinkInfo m_siteLinkInfo;
	SafeBuf m_mySiteLinkInfoBuf;
	SafeBuf m_myPageLinkInfoBuf;
	SafeBuf m_myTempLinkInfoBuf;
	char m_isInjecting;
	char m_isImporting;
	char m_useFakeMime;
	char m_useSiteLinkBuf;
	char m_usePageLinkBuf;
	char m_printInXml;
	SafeBuf m_tmpBuf11;
	SafeBuf m_tmpBuf12;
	Multicast m_mcast11;
	Multicast m_mcast12;
	// lists from cachedb for msg25's msg20 replies serialized
	RdbList m_siteReplyList;
	RdbList m_pageReplyList;
	MsgC m_msgc;
	bool m_isAllowed;
	bool m_forwardDownloadRequest;
	bool m_isChildDoc;
	Msg13 m_msg13;
	Msg13Request m_msg13Request;
	bool m_isSpiderProxy;
	// for limiting # of iframe tag expansions
	int32_t m_numExpansions;
	char m_newOnly;
	char m_isWWWDup;
	char m_calledMsg0b;

	SafeBuf m_linkSiteHashBuf;
	SafeBuf m_linkdbDataBuf;
	SafeBuf m_langVec;
	Msg0 m_msg0b;
	class RdbList *m_ulist;
	char     *m_linkInfoColl;
	SiteGetter m_siteGetter;
	int64_t  m_siteHash64;
	int32_t m_siteHash32;
	char *m_httpReply;
	char m_incrementedAttemptsCount;
	char m_incrementedDownloadCount;
	char m_redirectFlag;
	char m_spamCheckDisabled;
	char m_useRobotsTxt;
	int32_t m_robotsTxtLen;
	int32_t m_httpReplySize;
	int32_t m_httpReplyAllocSize;
	char *m_filteredContent;
	int32_t m_filteredContentLen;
	int32_t m_filteredContentAllocSize;
	int32_t m_filteredContentMaxSize;
	char m_calledThread;
	int32_t m_errno;
	int32_t m_hostHash32a;
	int32_t m_hostHash32b;
	int32_t m_domHash32;
	int32_t m_priorityQueueNum;

	// this points into m_msge0 i guess
	Msge0 m_msge0;

	// this points into m_msge1 i guess
	int32_t *m_outlinkIpVector;
	SafeBuf m_outlinkTagRecPtrBuf;
	SafeBuf m_fakeIpBuf;
	char m_hasNoIndexMetaTag;
	char m_hasUseFakeIpsMetaTag;
	Msge1 m_msge1;
	TagRec **m_outlinkTagRecVector;
	SafeBuf m_fakeTagRecPtrBuf;
	TagRec m_fakeTagRec;

	//
	// diffbot parms for indexing diffbot's json output
	//

	char *hashJSONFields ( HashTableX *table );
	char *hashJSONFields2 ( HashTableX *table , HashInfo *hi , Json *jp ,
				bool hashWithoutFieldNames ) ;

	char *hashXMLFields ( HashTableX *table );

	Json *getParsedJson();
	// object that parses the json
	Json m_jp;

	// related query algo stuff
	int64_t m_tlbufTimer;

	// flow flags

	bool m_computedMetaListCheckSum;

	// cachedb related args
	bool    m_allHashed;

	// for getRelatedDocIdsWithTitles() launching msg20s
	int32_t m_numMsg20Replies;
	int32_t m_numMsg20Requests;

	int8_t *m_outlinkHopCountVector;
	int32_t  m_outlinkHopCountVectorSize;
	char m_isFiltered;
	int32_t m_urlFilterNum;
	int32_t m_numOutlinksAdded;
	int32_t m_numOutlinksAddedFromSameDomain;
	int32_t m_numOutlinksFiltered;
	int32_t m_numOutlinksBanned;
	int32_t m_numRedirects;
	bool m_isPageParser;
	Url m_baseUrl;
	Msg20Reply m_reply;
	Msg20Request *m_req;
	char  m_linkTextBuf[MAX_LINK_TEXT_LEN];
	char m_surroundingTextBuf[MAX_SURROUNDING_TEXT_WIDTH];
	char m_rssItemBuf[MAX_RSSITEM_SIZE];
	SafeBuf m_gsbuf;
	char *m_note;
	char *m_imageUrl;
	char *m_imageUrl2;
	SafeBuf m_imageUrlBuf;
	SafeBuf m_imageUrlBuf2;
	MatchOffsets m_matchOffsets;
	Query m_query;
	Matches m_matches;
	// meta description buf
	int32_t m_dbufSize;
	char m_dbuf[1024];
	SafeBuf m_htb;
	Title m_title;
	Summary m_summary;
	char m_isCompromised;
	char m_isNoArchive;
	char m_isErrorPage;
	char m_isHijacked;

	// stuff
	char *m_statusMsg;
	Msg4  m_msg4;
	bool  m_incCount;
	bool  m_decCount;

	bool  m_deleteFromIndex;

	// ptrs to stuff
	SafeBuf m_titleRecBuf;
	key_t   m_titleRecKey;

	// for isDupOfUs()
	char *m_dupTrPtr;
	int32_t  m_dupTrSize;

	key_t     m_doledbKey;
	SpiderRequest m_sreq;
	SpiderReply   m_srep;//newsr;

	// bool flags for what procedures we have done
	bool m_checkedUrlFilters;
	
	bool m_listAdded                ;
	bool m_listFlushed              ;
	bool m_check1                   ;
	bool m_check2                   ;
	bool m_prepared                 ;
	bool m_updatedCounts            ;
	bool m_updatedCounts2           ;
	bool m_copied1                  ;
	bool m_updatingSiteLinkInfoTags ;

	int64_t m_calledMsg22d             ;
	bool m_didDelay                 ;
	bool m_didDelayUnregister       ;
	bool m_calledMsg22e             ;
	bool m_calledMsg22f             ;
	bool m_calledMsg25              ;
	bool m_calledMsg25b             ;
	bool m_calledMsg8b              ;
	bool m_calledMsg40              ;
	bool m_calledSections           ;
	bool m_firstEntry               ;
	bool m_firstEntry2              ;
	bool m_launchedSpecialMsg8a     ;
	bool m_launchedMsg8a2           ;
	bool m_loaded                   ;

	bool m_processedLang            ;

	bool m_doingConsistencyCheck ;

	int32_t    m_langIdScore;

	int32_t m_dist;

	// use to store a \0 list of "titles" of the root page so we can
	// see which if any are the venue name, and thus match that to
	// addresses of the venue on the site, and we can use those addresses
	// as default venue addresses when no venues are listed on a page
	// on that site.
	char   m_rootTitleBuf[ROOT_TITLE_BUF_MAX];
	int32_t   m_rootTitleBufSize;

	// . this is filtered
	// . certain punct is replaced with \0
	char   m_filteredRootTitleBuf[ROOT_TITLE_BUF_MAX];
	int32_t   m_filteredRootTitleBufSize;

	// like m_rootTitleBuf but for the current page
	char   m_titleBuf[ROOT_TITLE_BUF_MAX];
	int32_t   m_titleBufSize;


	bool m_setTr                    ;

	void (* m_masterLoop) ( void *state );
	void  * m_masterState;

	void (* m_callback1) ( void *state );	
	bool (* m_callback2) ( void *state );	
	void  *m_state;

	bool m_skipIframeExpansion;

	// this is non-zero if we decided not to index the doc
	int32_t m_indexCode;

	// the spider priority
	int32_t m_priority;

	// the download error, like ETIMEDOUT, ENOROUTE, etc.
	int32_t m_downloadStatus;

	// . when the download was completed. will be zero if no download done
	// . used to set SpiderReply::m_downloadEndTime because we need
	//   high resolution for that so we can dole out the next spiderrequest
	//   from that IP quickly if the sameipwait is like 500ms.
	int64_t m_downloadEndTime;

	int32_t  m_metaListAllocSize;
	char *m_p;
	char *m_pend;

	int32_t  m_maxCacheAge;

	char     *m_wikiqbuf;
	int32_t      m_wikiqbufSize;

	bool      m_registeredSleepCallback;
	bool      m_addedNegativeDoledbRec;
	
	bool          m_hashedTitle;
	bool          m_hashedMetas;

	int32_t          m_niceness;

	bool m_usePosdb     ;
	bool m_useClusterdb ;
	bool m_useLinkdb    ;
	bool m_useSpiderdb  ;
	bool m_useTitledb   ;
	bool m_useTagdb     ;
	bool m_usePlacedb   ;
	bool m_useSecondaryRdbs ;

	SafeBuf *m_pbuf;
	// used by SpiderLoop to set m_pbuf to
	SafeBuf  m_sbuf;
	// store termlist into here if non-null
	bool     m_storeTermListInfo;
	char     m_sortTermListBy;

	// store the terms that we hash into this table so that PageParser.cpp
	// can print what was hashed and with what score and what description
	class HashTableX *m_wts;
	HashTableX m_wtsTable;
	SafeBuf m_wbuf;

	// Msg25.cpp stores its pageparser.cpp output into this one
	SafeBuf m_pageLinkBuf;
	SafeBuf m_siteLinkBuf;

	// which set() function was called above to set us?
	bool          m_setFromTitleRec;
	bool          m_setFromSpiderRec;
	bool          m_setFromUrl;
	bool          m_setFromDocId;
	bool          m_freeLinkInfo1;
	bool          m_freeLinkInfo2;
	bool          m_contentInjected;

	bool          m_recycleContent;

	char *m_rawUtf8Content;
	int32_t  m_rawUtf8ContentSize;
	int32_t  m_rawUtf8ContentAllocSize; // we overallocate sometimes
	char *m_expandedUtf8Content;
	int32_t  m_expandedUtf8ContentSize;
	char *m_savedp;
	char *m_oldp;
	bool  m_didExpansion;
	SafeBuf m_esbuf;
	SafeBuf m_xbuf;

	// used by msg13
	class Msg13Request *m_r;

	// Msg20 uses this to stash its TcpSlot
	void *m_slot;

	char *getTestDir();

	bool m_freed;

	bool m_msg4Waiting;
	bool m_msg4Launched;

	// word spam detection
	char *getWordSpamVec ( );
	bool setSpam ( int32_t *profile, int32_t plen , int32_t numWords , 
		       unsigned char *spam );
	int32_t  getProbSpam  ( int32_t *profile, int32_t plen , int32_t step );
	bool m_isRepeatSpammer;
	int32_t m_numRepeatSpam;
	bool m_totallySpammed;

	// frag vector (repeated fragments). 0 means repeated, 1 means not.
	// vector is 1-1 with words in the document body.
	char *getFragVec ( );

	bool injectDoc ( char *url ,
			 class CollectionRec *cr ,
			 char *content ,
			 bool contentHasMime ,
			 int32_t hopCount,
			 int32_t charset,

			 bool deleteUrl,
			 char *contentTypeStr, // text/html, text/xml etc.
			 bool spiderLinks ,
			 char newOnly, // index iff new

			 void *state,
			 void (*callback)(void *state) ,

			 uint32_t firstIndexedTime = 0,
			 uint32_t lastSpideredDate = 0 ,
			 int32_t  injectDocIp = 0 );


	bool injectLinks  ( HashTableX *linkDedupTable ,
			    HashTableX *domDedupTable ,
			    void *finalState , 
			    void (* finalCallback)(void *));

	int64_t logQueryTimingStart();
	void logQueryTimingEnd(const char* function, int64_t startTime);

	bool doInjectLoop ( );
	void doneInjecting ( class XmlDoc *xd );
	int32_t  m_i;
	int32_t  m_blocked;
	void *m_finalState;
	void (* m_finalCallback) ( void *state );
	int64_t m_cacheStartTime;
};

// . PageParser.cpp uses this class for printing hashed terms out by calling
//   XmlDoc::print()
// . we store TermInfos into XmlDoc::m_wtsTable, a HashTableX
// . one for each term hashed
// . the key is the termId. dups are allowed
// . the term itself is stored into a separate buffer, m_wbuf, a SafeBuf, so
//   that TermInfo::m_term will reference that and it won't disappear on us
class TermDebugInfo {
 public:
	int32_t      m_termOff;
	int32_t      m_termLen;
	int32_t      m_descOff;   // the description offset
	int32_t      m_prefixOff; // the prefix offset, like "site" or "gbadid"
	int64_t m_termId;
	int32_t      m_date;
	bool      m_shardByTermId;

	char      m_langId;
	char      m_diversityRank;
	char      m_densityRank;
	char      m_wordSpamRank;
	char      m_hashGroup;
	int32_t      m_wordNum;
	int32_t      m_wordPos;
	POSDBKEY  m_key; // key144_t
	// 0 = not a syn, 1 = syn from presets,2=wikt,3=generated
	char      m_synSrc;
	int64_t  m_langBitVec64;
};


extern uint8_t score32to8 ( uint32_t score ) ;


// . returns 0 to 100 , the probability of spam for this subprofile
// . a "profile" is an array of all the positions of a word in the document
// . a "position" is just the word #, like first word, word #8, etc...
// . we are passed a subprofile, "profile", of the actual profile
//   because some of the document may be more "spammy" than other parts
// . inlined to speed things up because this may be called multiple times
//   for each word in the document
// . if "step" is 1 we look at every       word position in the profile
// . if "step" is 2 we look at every other word position 
// . if "step" is 3 we look at every 3rd   word position, etc...
inline int32_t XmlDoc::getProbSpam(int32_t *profile, int32_t plen, int32_t step) {

	// you can spam 2 or 1 letter words all you want to
	if ( plen <= 2 ) return 0;

	// if our step is bigger than the profile return 0
	if ( step == plen ) return 0;

	register int32_t avgSpacing, stdDevSpacing;
	int32_t d,dev=0;
	register int32_t i;
	
	for (int32_t j = 0; j < step; j++) {

		// find avg. of gaps between consecutive tokens in subprofile
		// TODO: isn't profile[i] < profile[i+1]??
		int32_t istop = plen-1;
		avgSpacing = 0;
		for (i=0; i < istop; i += step ) 
			avgSpacing += ( profile[i] - profile[i+1] ); 
		// there's 1 less spacing than positions in the profile
		// so we divide by plen-1
		avgSpacing = (avgSpacing * 256) / istop;

		// compute standard deviation of the gaps in this sequence
		stdDevSpacing = 0;
		for (i = 0 ; i < istop; i += step ) {
			d = (( profile[i] - profile[i+1]) * 256 ) - avgSpacing;
			if ( d < 0 ) stdDevSpacing -= d;
			else         stdDevSpacing += d;
		}

		// TODO: should we divide by istop-1 for stdDev??
		stdDevSpacing /= istop;

		// average of the stddevs for all sequences
		dev += stdDevSpacing;
	}

	dev /= step;
	
	// if the plen is big we should expect dev to be big
	// here's some interpolation points:
	// plen >=  2  and  dev<= 0.2  --> 100%
	// plen  =  7  and  dev = 1.0  --> 100%
	// plen  = 14  and  dev = 2.0  --> 100%
	// plen  = 21  and  dev = 3.0  --> 100%
	// plen  = 7   and  dev = 2.0  -->  50%

	// NOTE: dev has been multiplied by 256 to avoid using floats
	if ( dev <= 51.2 ) return 100;  // (.2 * 256)
	int32_t prob = ( (256*100/7) * plen ) / dev;

	if (prob>100) prob=100;

	return prob;
}

#endif

