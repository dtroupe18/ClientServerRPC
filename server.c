//
// Created by Dave on 2/26/17.
//

#include        <sys/types.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <unistd.h>
#include        <errno.h>
#include        <string.h>
#include        <sys/socket.h>
#include        <netdb.h>
#include        <pthread.h>

// Miniature server to exercise getaddrinfo(2).

int
claim_port( const char * port ) // prepare to get information
{
    struct addrinfo         addrinfo;
    struct addrinfo *       result;
    int                     sd;
    char                    message[256];
    int                     on = 1;
    
    // THIS IS ALL SETUP FOR THE CONNECTION
    addrinfo.ai_flags = AI_PASSIVE;         // for bind()
    addrinfo.ai_family = AF_INET;           // IPv4 only
    addrinfo.ai_socktype = SOCK_STREAM;     // Want TCP/IP
    addrinfo.ai_protocol = 0;               // Any protocol
    addrinfo.ai_addrlen = 0;
    addrinfo.ai_addr = NULL;
    addrinfo.ai_canonname = NULL;
    addrinfo.ai_next = NULL;
    if ( getaddrinfo( 0, port, &addrinfo, &result ) != 0 )          // want port 8675
    {
        write( 2, message, sprintf( message, "\x1b[1;31mgetaddrinfo( %s ) failed.  File %s line %d.\x1b[0m\n", port, __FILE__, __LINE__ ) );
        return -1;
    }
    else if ( (sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol )) == -1 )
    {
        write( 2, message, sprintf( message, "socket() failed.  File %s line %d.\n", __FILE__, __LINE__ ) );
        freeaddrinfo( result );
        return -1;
    }
    else if ( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) == -1 )
    {
        write( 2, message, sprintf( message, "setsockopt() failed.  File %s line %d.\n", __FILE__, __LINE__ ) );
        freeaddrinfo( result );
        close( sd );
        return -1;
    }
    else if ( bind( sd, result->ai_addr, result->ai_addrlen ) == -1 )
    {
        write( 2, message, sprintf( message, "\x1b[2;31mbind() to port %s failed.  File %s line %d \x1b[0m\n", port, __FILE__, __LINE__ ) );
        freeaddrinfo( result );
        close( sd );
        return -1;
    }
    else
    {
        write( 2, message, sprintf( message,  "\x1b[1;32mBind to port %s succeeded.\x1b[0m\n", port ) );
        freeaddrinfo( result );
        return sd;                      // bind() succeeded;
    }
}

void *
client_session_thread( void * arg )
{
    int                     sd;
    char                    request[2048];
    char                    response[2048];
    char                    message[2048];
    char                    temp;
    int                     i;
    int                     limit, size;
    float                   ignore;
    long                    senderIPaddr;
    int n;
    
    sd = *(int *)arg;
    free( arg );                                    // keeping to memory management covenant
    pthread_detach( pthread_self() );               // Don't join on this thread
    
    while ( read( sd, request, sizeof(request) ) > 0 ) // READ INFO FROM CLIENT
    {
        size = strlen( request );
        for (int i = 0 ; i < size - 1 ; i++ )
        {
            temp = request[i];
            request[i] = request[i + 1];
            request[i + 1] = temp;
        }
        
        //printf("%s\n", request);
        n = write( sd, request, strlen(request) + 1 ); // SEND ROTATED STRING BACK
        if (n < 0) {
            printf("Failed to write to socket"); // Error check to make sure they information was sent back
        }
    }
    close(sd);
    return 0;
    
    
}

int
main( int argc, char ** argv )
{
    int                     sd;
    char                    message[256];
    pthread_t               tid;
    pthread_attr_t          kernel_attr;
    socklen_t               ic;
    int                     fd;
    struct sockaddr_in      senderAddr;
    int *                   fdptr;
    
    
    // this is all multi threading that isn't required.
    if ( pthread_attr_init( &kernel_attr ) != 0 )
    {
        write( 2, message, sprintf( message, "pthread_attr_init() failed in file %s line %d\n", __FILE__, __LINE__ ) );
        return 0;
    }
    else if ( pthread_attr_setscope( &kernel_attr, PTHREAD_SCOPE_SYSTEM ) != 0 )
    {
        write( 2, message, sprintf( message, "pthread_attr_setscope() failed in file %s line %d\n", __FILE__, __LINE__ ) );
        return 0;
    }
    else if ( (sd = claim_port( "8675" )) == -1 ) // starts here
    {
        write( 2, message, sprintf( message,  "\x1b[1;31mCould not bind to port %s errno %s\x1b[0m\n", "8675", strerror( errno ) ) );
        return 1;
    }
    else if ( listen( sd, 100 ) == -1 )
    {
        write( 2, message, sprintf( message, "listen() failed in file %s line %d\n", __FILE__, __LINE__ ) );
        close( sd );
        return 0;
    }
    else
    {
        ic = sizeof(senderAddr);
        while ( (fd = accept( sd, (struct sockaddr *)&senderAddr, &ic )) != -1 )
            // fd to client discripter
        {
            fdptr = (int *)malloc( sizeof(int) );
            *fdptr = fd;             // pointers are not the same size as ints any more.
            if ( pthread_create( &tid, &kernel_attr, client_session_thread, fdptr ) != 0 )
            {
                printf( "pthread_create() failed in file %s line %d\n", __FILE__, __LINE__ );
                return 0;
            }
            else
            {
                continue;
            }
        }
        close( sd );
        return 0;
    }
}
