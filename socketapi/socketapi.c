

/**
 * This function sets an SRH in the sfd containing the table of segments.
 * The destination segment is inserted by the function and therefore is not
 * needed in the parameter.
 * The number of segments is given by segment_number.
 * The first segment in segment table will be in the first segment used.
 */
int set_srv6_segments(int sfd, char *segments[], size_t segment_number)
{
	srh_len = sizeof(*srh) + (segment_number + 1) * sizeof(struct in6_addr);
	srh = malloc(srh_len);
	if (!srh) {
		fprintf(stderr, "Out of memory\n");
		err = -1;
		goto out;
	}

	srh->nexthdr = 0;
	srh->hdrlen = 4;
	srh->type = 4;
	srh->segments_left = segment_number;
	srh->first_segment = srh->segments_left;
	srh->flag_1 = 0;
	srh->flag_2 = 0;
	srh->reserved = 0;
	memset(&srh->segments[0], 0, sizeof(struct in6_addr)); // Final destination segment

	for (int i = 0; i < segment_number; i++)
		inet_pton(AF_INET6, segments[i], &srh->segments[segment_number-i]);

	err = setsockopt(fd, IPPROTO_IPV6, IPV6_RTHDR, srh, srh_len);
	if (err < 0) {
		perror("sr_socket - setsockopt");
		return -1;
	}

	return 0;
}

