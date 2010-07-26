/*
 * TCP_checksum.c -  compute a TCP pseudo-header checksum
 *
 *  Created on: Jun 3, 2010
 *      Author: rado
 */


TCP_checksum(struct ip_pkt *ip_packet, unsigned len)
{
	tcp_sgm		*tcp_segment = (tcp_sgm *)ip_packet->ip_data;
	unsigned	short	*sptr;
	unsigned	long checksum;
	unsigned	i;

	checksum = 0;

	sptr = (unsigned short *) &ip_packet->ip_src;
	/* 2*IP_ALEN octets = IP_ALEN shorts... */
	/* they are in net order.				*/
	for (i=0; i<IP_ALEN; ++i)
		checksum += *sptr++;
	sptr = (unsigned short *)tcp_segment;
	checksum += hs2net(IPT_TCP + len);
	if (len % 2) {
		((char *)ptcp)[len] = 0;	/* pad */
		len += 1;	/* for the following division */
	}
	len >>= 1;	/* convert to length in shorts */

	for (i=0; i<len; i++)
		checksum += *sptr++;
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	return (short)(~checksum & 0xffff);
}
