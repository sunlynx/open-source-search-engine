#include "gb-include.h"

#include "Dir.h"

Dir::Dir ( ) {
	m_dirname = NULL;
	m_dir     = NULL;
	m_needsClose = false;
}


Dir::~Dir ( ) {
	reset();
}

void Dir::reset ( ) {
	close();
	if ( m_dirname ) free ( m_dirname );
	m_dirname = NULL;
}

bool Dir::set ( const char *d1, const char *d2 ) {
	reset ();
	char tmp[1024];
	if ( gbstrlen(d1) + gbstrlen(d2) + 1 > 1024 ) {
		log("disk: Could not set directory, directory name \"%s/%s\" "
		    "is too big.",d1,d2);
		return false;
	}
	sprintf ( tmp , "%s/%s", d1 , d2 );
	return set ( tmp );
}

bool Dir::set ( const char *dirname ) {
	reset ();
	m_dirname = strdup ( dirname );
	if ( m_dirname ) return true;
	log("disk: Could not set directory, directory name to \"%s\": %s.",
	    dirname,mstrerror(g_errno));
	return false;
}

bool Dir::close ( ) {
	if ( m_dir && m_needsClose ) closedir ( m_dir );
	m_needsClose = false;
	return true;
}

bool Dir::open ( ) {
	close ( );
	if ( ! m_dirname ) return false;
 retry8:
	// opendir() calls malloc
	g_inMemFunction = true;
	m_dir = opendir ( m_dirname );
	g_inMemFunction = false;
	// interrupted system call
	if ( ! m_dir && errno == EINTR ) goto retry8;

	if ( ! m_dir ) 
		g_errno = errno;

	if ( ! m_dir ) 
		return log("disk: opendir(%s) : %s",
			   m_dirname,strerror( g_errno ) );
	m_needsClose = true;
	return true;
}

int Dir::getNumFiles ( char *pattern ) {
	int count = 0;
	while ( getNextFilename ( pattern ) ) count++;
	return count;
}

char *Dir::getNextFilename ( char *pattern ) {

	if ( ! m_dir ) {
		log("dir: m_dir is NULL so can't find pattern %s",pattern);
		return NULL;
	}

	struct dirent *ent;
	int32_t plen = gbstrlen ( pattern );
	while ( (ent = readdir ( m_dir ))  ) {
		char *filename = ent->d_name;
		if ( ! pattern ) return filename;
		if ( plen>2 && pattern[0] == '*' && pattern[plen-1] == '*' ) {
			pattern[plen-1]='\0';
			char *s = strstr ( filename , pattern+1 ) ;
			pattern[plen-1]='*';
			if ( ! s ) continue;
			else       return filename;
		}
		if ( pattern[0] == '*' ) {
			if ( gbstrlen(filename) < gbstrlen(pattern + 1) ) continue;
			char *tail = filename + 
				gbstrlen ( filename ) - 
				gbstrlen ( pattern ) + 1;
			if ( strcmp ( tail , pattern+1) == 0 ) return filename;
		}
		if ( pattern[plen-1]=='*' ) {
			if ( strncmp ( filename , pattern , plen - 1 ) == 0 )
				return filename;
		}
	}

	return NULL;
}

// . replace the * in the pattern with a unique id from getNewId()
char *Dir::getFullName ( char *filename ) {
	static char buf[1024];
	sprintf ( buf , "%s/%s", m_dirname , filename );
	return buf;
}
