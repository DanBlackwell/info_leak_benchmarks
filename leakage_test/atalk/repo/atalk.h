#ifndef _ATALK_H__
#define _ATALK_H__

/*
 * AppleTalk networking structures
 *
 * The following are directly referenced from the University Of Michigan
 * netatalk for compatibility reasons.
*/

#include <stdio.h>
#include <cstring>

#define ENOBUF		       132	// No buffer space available 
#define ENOTCON		       134	// Transport endpoint is not connected 
#define  __be16            unsigned short
#define  TCP_ESTABLISHED   5
#define ALEN               16
struct atalk_sock 
{
	unsigned char	dst_node;    // 8 bits
	unsigned char	src_node;    // 8 bits
	unsigned char	dst_port;    // 8 bits
	unsigned char	src_port;    // 8 bits
	int sk_state;                // 32 bits  
	char res[ALEN];
	
	struct atalk_sock& operator=(const struct atalk_sock& that)
    {
        dst_node = that.dst_node;
        src_node  = that.src_node;
        dst_port = that.dst_port;
        src_port  = that.src_port;
        sk_state  = that.sk_state;
        
        strncpy(res, that.res, ALEN);
        
        return *this;
    }
} ;
typedef struct atalk_sock *atptr;


static inline bool sock_flag(const atptr sk)
{
	return 0;
}

int atalk_getname(atptr sock, atptr &uaddr, int peer);

#endif /* _ATALK_H__ */

