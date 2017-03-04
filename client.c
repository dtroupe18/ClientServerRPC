//
// Created by Dave on 2/27/17.
//

#include        <sys/types.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <unistd.h>
#include        <errno.h>
#include        <string.h>
#include        <sys/socket.h>
#include        <netdb.h>
#include <stdbool.h>

// Miniature client to exercise getaddrinfo(2).

int
connect_to_server(const char * server, const char * port)
{
    
    int                     sd;
    struct addrinfo         addrinfo;
    struct addrinfo *       result;
    char                    message[256];
    
    addrinfo.ai_flags = 0;
    addrinfo.ai_family = AF_INET;           // IPv4 only
    addrinfo.ai_socktype = SOCK_STREAM;     // Want TCP/IP
    addrinfo.ai_protocol = 0;               // Any protocol
    addrinfo.ai_addrlen = 0;
    addrinfo.ai_addr = NULL;
    addrinfo.ai_canonname = NULL;
    addrinfo.ai_next = NULL;
    if ( getaddrinfo( server, port, &addrinfo, &result ) != 0 )
    {
        fprintf( stderr, "\x1b[1;31mgetaddrinfo( %s ) failed.  File %s line %d.\x1b[0m\n", server, __FILE__, __LINE__ );
        return -1;
    }
    else if ( errno = 0, (sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol )) == -1 )
    {
        freeaddrinfo( result );
        return -1;
    }
    else
    {
        do {
            if ( errno = 0, connect( sd, result->ai_addr, result->ai_addrlen ) == -1 )
            {
                sleep( 1 );
                write( 1, message, sprintf( message, "\x1b[2;33mConnecting to server %s ...\x1b[0m\n", server ) );
            }
            else
            {
                freeaddrinfo( result );
                return sd;              // connect() succeeded
            }
        } while ( errno == ECONNREFUSED );
        freeaddrinfo( result );
        return -1;
    }
}

int
main( int argc, char ** argv )
{
    int                     sd;
    char                    message[256];
    char                    string[512] = "ABCDEF";
    char                    buffer[512];
    int                     len;
    int n;
    static int oneTime = 0;
    bool done = false;
    int count = 0;
    
    if ( argc < 2 )
    {
        fprintf( stderr, "\x1b[1;31mNo host name specified.  File %s line %d.\x1b[0m\n", __FILE__, __LINE__ );
        exit( 1 );
    }
    else if ( (sd = connect_to_server( argv[1], "8675" )) == -1 )
    {
        write( 1, message, sprintf( message,  "\x1b[1;31mCould not connect to server %s errno %s\x1b[0m\n", argv[1], strerror( errno ) ) );
        return 1;
    }
    else
    {
        printf( "Connected to server %s\n", argv[1] );
        
        
        while(!done) {
            count++;
            if (count == 1000) {
                done = true;
                printf("%s", "String rotation complete\n");
                close(sd); // close the socket
                exit(1); // close program
            }
            
            if (oneTime == 1) {
                n = read(sd, buffer, sizeof(buffer)); // read information from server
                if (n < 0) {
                    perror("Error reading from socket"); // server did not reply back or disconnected
                    exit(1); // close program
                }
                
                else {
                    printf("%s\n", buffer);
                    write(sd, buffer, strlen(buffer)); // write information back to the server to be rotated
                }
            }
            
            else { // add the trailing \0 to the string
                len = strlen(string);
                string[len+1]= '\0';
                printf("%s\n", buffer);
                // usleep(500);
                write( sd, string, strlen(string ) + 1 ); // write string only because the server hasn't sent anything yet
                oneTime = 1;
            }
        }
        close(sd);
        return 0;
        
    }
}
