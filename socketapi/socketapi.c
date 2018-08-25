#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/seg6.h>

/**
 * This function sets an SRH in the sfd containing the table of segments.
 * The destination segment is inserted by the function and therefore is not
 * needed in the parameter.
 * The number of segments is given by segment_number.
 * The first segment in segment table will be in the first segment used.
 */
int set_srv6_segments(int sfd, char *segments[], size_t segment_number)
{
	struct ipv6_sr_hdr *srh;
	size_t srh_len = sizeof(*srh) + (segment_number + 1) * sizeof(struct in6_addr);
	srh = malloc(srh_len);
	if (!srh) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}

	srh->nexthdr = 0;
	srh->hdrlen = 2*(segment_number + 1);
	srh->type = 4;
	srh->segments_left = segment_number;
	srh->first_segment = srh->segments_left;
	srh->flags = 0;
	srh->tag = 0;
	memset(&srh->segments[0], 0, sizeof(struct in6_addr)); // Final destination segment

	for (size_t i = 0; i < segment_number; i++) {
		if (inet_pton(AF_INET6, segments[i], &srh->segments[segment_number-i]) != 1) {
			fprintf(stderr, "Cannot parse %s as an IPv6 address\n", segments[i]);
			free(srh);
			return -1;
		}
	}

	if (setsockopt(sfd, IPPROTO_IPV6, IPV6_RTHDR, srh, srh_len) < 0) {
		perror("sr_socket - setsockopt");
		free(srh);
		return -1;
	}

	free(srh);
	return 0;
}

int main(int argc, char *argv[])
{
	int sfd;
	if (argc < 4) {
		fprintf(stderr, "Usage: %s host port segment1 [segment2 ...]\n", argv[0]);
		return -1;
	}

	struct sockaddr_in6 sin6;
	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;
	sin6.sin6_port = htons(atoi(argv[2]));
	if (inet_pton(AF_INET6, argv[1], sin6.sin6_addr.s6_addr) != 1) {
		fprintf(stderr, "Cannot convert parse %s as an IPv6 address\n", argv[1]);
		return -1;
	}

	sfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sfd == -1) {
		perror("Cannot create socket");
		return -1;
	}

	if (set_srv6_segments(sfd, &argv[3], argc - 3) == -1) {
		fprintf(stderr, "Cannot set SRH\n");
		return -1;
	}

	if (connect(sfd, (struct sockaddr *) &sin6, sizeof(sin6)) == -1) {
		perror("Cannot connect to server");
		return -1;
	}

	char buf [] = "Hello with Segment Routing";
	if (write(sfd, buf, sizeof(buf)) != sizeof(buf)) {
		perror("Cannot write on the socket");
		close(sfd);
		return -1;
	}

	if (close(sfd) == -1) {
		perror("Cannot close socket\n");
		return -1;
	}

	printf("Success !\n");

	return 0;
}

