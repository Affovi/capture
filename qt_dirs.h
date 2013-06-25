#ifndef QT_DIRS_H
#define QT_DIRS_H

#include <QString>
#include <QDir>

#define toNativeSeparators( s ) QDir::toNativeSeparators ( s )

static inline QString removeTrailingSlash( QString s )
{
    if( ( s.length() > 1 ) && ( s[s.length()-1] == QLatin1Char( '/' ) ) )
        s.remove( s.length() - 1, 1 );
    return s;
}

#define savedirpathFromFile( a ) p_intf->p_sys->filepath = toNativeSeparators( QFileInfo( a ).path() )
#define toNativeSepNoSlash( a ) toNativeSeparators( removeTrailingSlash( a ) )

static inline QString colon_escape( QString s )
{
    return s.replace( ":", "\\:" );
}
static inline QString colon_unescape( QString s )
{
    return s.replace( "\\:", ":" ).trimmed();
}

#endif
