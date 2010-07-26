/*
 * tcp.h
 *
 *  Created on: Jun 3, 2010
 *      Author: rado
 */

#ifndef TCP_H_
#define TCP_H_

/* Constants */
#define LITTLE_ENDIAN		 /* Comment out in case machine is big endian		*/
#define	EP_DLEN		1500	 /* Maximum length of data field in Ethernet packet	*/
#define	IP_ALEN	4			 /* IP address length in bytes (octets)				*/
typedef	unsigned long IPaddr;/* Internet address								*/
#define	IPT_TCP		6		 /* protocol type for TCP packets					*/

/* Data type definitions*/
typedef long tcpseq;

struct ip_pkt
{
	u_char ip_verlen; 	/* IP version & header length (in longs)*/
	u_char ip_tos; 		/* type of service						*/
	u_short ip_len; 	/* total packet length (in octets)		*/
	short ip_id; 		/* datagram id							*/
	short ip_fragoff; 	/* fragment offset (in 8-octet's)		*/
	u_char ip_ttl; 		/* time to live, in gateway hops		*/
	u_char ip_proto; 	/* IP protocol (see IPT_* above)		*/
	short ip_cksum; 	/* header checksum 						*/
	IPaddr ip_src; 		/* IP address of source					*/
	IPaddr ip_dst; 		/* IP address of destination			*/
	u_char ip_data[1]; 	/* variable length data					*/
}

/* Data type for holding TCP header */
typedef struct
{
	unsigned short tcp_sport;	/* source port				*/
	unsigned short tcp_dport;	/* destination port			*/
	tcpseq tcp_seq;				/* sequence					*/
	tcpseq tcp_ack; 			/* acknowledged sequence	*/
	unsigned char tcp_offset;
	unsigned char tcp_code; 	/* control flags			*/
	unsigned short tcp_window; 	/* window advertisement		*/
	unsigned short tcp_cksum; 	/* check sum				*/
	unsigned short tcp_urgptr; 	/* urgent pointer			*/
} tcp_hdr;

/* Data type for holding TCP segment (datagram) */
typedef struct
{
	tcp_hdr tcp_header;
	unsigned char tcp_data[1];
} tcp_sgm;

/* TCP Control Bits */
#define	TCPF_URG	0x20	/* urgent pointer is valid		*/
#define	TCPF_ACK	0x10	/* acknowledgment field is valid	*/
#define	TCPF_PSH	0x08	/* this segment requests a push		*/
#define	TCPF_RST	0x04	/* reset the connection			*/
#define	TCPF_SYN	0x02	/* synchronize sequence numbers		*/
#define	TCPF_FIN	0x01	/* sender has reached end of its stream	*/

/*Macros*/
#ifdef LITTLE_ENDIAN
#define hs2net(x) (unsigned) ((((x)>>8) &0xff) | (((x) & 0xff)<<8))
#define	net2hs(x) hs2net(x)

#define hl2net(x)	(((((x)& 0xff)<<24) | ((x)>>24) & 0xff) | \
	(((x) & 0xff0000)>>8) | (((x) & 0xff00)<<8))
#define net2hl(x) hl2net(x)

#else
#define hs2net(x) (x)
#define net2hs(x) (x)
#define hl2net(x) (x)
#define net2hl(x) (x)
#endif

#endif /* TCP_H_ */
